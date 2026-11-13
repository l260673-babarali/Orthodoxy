#include "../Classes/Player.h"
#include<iostream>


Player::Player() : sprite(sheet_texture)
{
    shape.setSize({ 50.f, 80.f });

    // Spawn on the monastery's ground
    shape.setPosition({ 100.f, 200.f });

    if (!sheet_texture.loadFromFile(
        "Assets/Sprites/Player/knight_sheet.png"))
    {
        std::cout << "Could not load knight_sheet.png\n";
    }

    sprite.setTexture(sheet_texture);
    sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 60, 56 }));
}


// Runs every frame: reads the keys, moves the knight, handles collisions and picks the animation
void Player::update(
    float delta_time, const std::vector<sf::RectangleShape>& solids,
    float world_width
)
{
    const float speed = 200.f;
    const float gravity = 981.f;
    const float jump_velocity = -550.f;         // negative = up
    const float double_jump_velocity = -480.f;

    // ---- Count the timers down ----
    if (invincible_timer > 0.f)     invincible_timer -= delta_time;
    if (dash_cooldown_timer > 0.f)   dash_cooldown_timer -= delta_time;
    if (strike_cooldown_timer > 0.f) strike_cooldown_timer -= delta_time;

    if (is_striking)
    {
        strike_timer += delta_time;
        if (strike_timer >= strike_duration)
            is_striking = false;
    }

    // ---- Raw input (edge-detected) ----
    // "Pressed" = went down this frame. "Held" = is down right now.
    bool strike_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J);
    bool strike_pressed = strike_held && !strike_was_held;
    strike_was_held = strike_held;

    bool jump_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    bool jump_pressed = jump_held && !jump_was_held;
    jump_was_held = jump_held;

    float input_dir = 0.f;   // -1 = left, 0 = nothing, 1 = right
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) input_dir -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) input_dir += 1.f;

    // Input buffers: lets jump + strike count as "together" if pressed within a few ms
    if (jump_buffer_timer > 0.f)   jump_buffer_timer -= delta_time;
    if (strike_buffer_timer > 0.f) strike_buffer_timer -= delta_time;
    if (jump_pressed)   jump_buffer_timer = input_buffer_time;
    if (strike_pressed) strike_buffer_timer = input_buffer_time;

    // ---- Wall detection / cling state ----
    float wall_side = detectWall(solids);   // -1 wall on the left, 1 wall on the right, 0 no wall
    bool touching_wall = (wall_side != 0.f) && !is_on_ground;

    // Let go of the wall if we left it, or if we push away from it
    if (is_clinging)
    {
        bool moving_away = (input_dir != 0.f && input_dir == -wall_dir);
        if (!touching_wall || moving_away)
            is_clinging = false;
    }

    // Climb = jump and strike pressed together while touching a wall
    bool pressed_this_frame = jump_pressed || strike_pressed;
    bool both_pressed_together = jump_buffer_timer > 0.f && strike_buffer_timer > 0.f;
    bool not_pushing_away_from_wall = (input_dir != -wall_side);
    bool climb_pressed = touching_wall
        && pressed_this_frame
        && both_pressed_together
        && not_pushing_away_from_wall
        && !is_dashing;

    if (climb_pressed)
    {
        is_clinging = true;
        wall_dir = wall_side;
        facing_direction = wall_side;
        velocity_y = climb_velocity;
        is_striking = false;
        jump_buffer_timer = 0.f;
        strike_buffer_timer = 0.f;
    }

    // Normal jump/strike are disabled while at the wall, so they don't fire on the way to a climb
    bool near_wall = touching_wall || is_clinging;

    // ---- Strike ----
    if (strike_pressed && !is_striking && !is_dashing && !near_wall && strike_cooldown_timer <= 0.f)
    {
        is_striking = true;
        strike_timer = 0.f;
        strike_cooldown_timer = strike_cooldown;
        strike_id++;   // new swing = new number, so an enemy can be hit once by each swing
    }

    // ---- Crouch ----
    bool crouch_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    bool want_crouch = crouch_held && is_on_ground && !is_clinging;

    if (want_crouch && !is_crouching)
    {
        // Shrink the box, but keep the feet in the same spot
        float bottom = shape.getPosition().y + shape.getSize().y;
        shape.setSize({ hitbox_width, crouch_height });
        shape.setPosition({ shape.getPosition().x, bottom - crouch_height });
        is_crouching = true;
    }
    else if (!want_crouch && is_crouching)
    {
        // Only stand up if there is room above our head
        if (canStandUp(solids))
        {
            float bottom = shape.getPosition().y + shape.getSize().y;
            shape.setSize({ hitbox_width, stand_height });
            shape.setPosition({ shape.getPosition().x, bottom - stand_height });
            is_crouching = false;
        }
    }

    // ---- Horizontal input ----
    // move_x = how many pixels to move sideways this frame
    float move_x = 0.f;
    if (input_dir < 0.f)
    {
        move_x -= speed * delta_time;
        if (!is_clinging) facing_direction = -1.f;
    }
    if (input_dir > 0.f)
    {
        move_x += speed * delta_time;
        if (!is_clinging) facing_direction = 1.f;
    }

    if (is_striking && is_on_ground)
        move_x *= 0.3f;   // slow down while swinging on the ground

    if (is_crouching)
        move_x *= crouch_speed_multiplier;

    // Being pushed back after getting hit
    if (knockback_timer > 0.f)
    {
        move_x += knockback_push_x * delta_time;
        knockback_timer -= delta_time;
    }

    // ---- Jump ----
    if (is_on_ground)
    {
        coyote_timer = coyote_time;       // refill the "just walked off a ledge" grace time
        air_jumps_left = max_air_jumps;
    }
    else
    {
        coyote_timer -= delta_time;
    }

    if (jump_pressed && !is_dashing && !near_wall)
    {
        if (coyote_timer > 0.f)
        {
            // Normal jump (from the ground or just after leaving it)
            velocity_y = jump_velocity;
            coyote_timer = 0.f;
            is_on_ground = false;
        }
        else if (air_jumps_left > 0)
        {
            // Double jump in the air
            velocity_y = double_jump_velocity;
            air_jumps_left--;
        }
    }

    // ---- Dash ----
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        && can_dash && !is_dashing && !is_striking && !is_clinging && dash_cooldown_timer <= 0.f)
    {
        is_dashing = true;
        can_dash = false;
        dash_timer = dash_duration;
        dash_cooldown_timer = dash_cooldown;
        velocity_y = 0.f;
    }

    if (is_dashing)
    {
        // During a dash we fly straight, with no gravity
        move_x = facing_direction * dash_speed * delta_time;
        velocity_y = 0.f;
        dash_timer -= delta_time;
        if (dash_timer <= 0.f)
            is_dashing = false;
    }
    else
    {
        velocity_y += gravity * delta_time;
        // Clinging: gravity still pulls us down, but only as a slow slide
        if (is_clinging && velocity_y > wall_slide_speed)
            velocity_y = wall_slide_speed;
    }

    // ---- Horizontal movement + collision ----
    // Move first, then fix it: if we ended up inside a solid, push back out
    shape.move({ move_x, 0.f });
    if (move_x != 0.f)
    {
        for (const auto& solid : solids)
        {
            sf::FloatRect player_box = shape.getGlobalBounds();
            sf::FloatRect solid_box = solid.getGlobalBounds();
            if (player_box.findIntersection(solid_box))
            {
                if (move_x > 0.f)   // moving right: stop at the solid's left side
                    shape.setPosition({ solid_box.position.x - player_box.size.x, player_box.position.y });
                else               // moving left: stop at the solid's right side
                    shape.setPosition({ solid_box.position.x + solid_box.size.x, player_box.position.y });
                is_dashing = false;
            }
        }
    }

    // ---- Vertical movement + collision ----
    shape.move({ 0.f, velocity_y * delta_time });
    for (const auto& solid : solids)
    {
        sf::FloatRect player_box = shape.getGlobalBounds();
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (player_box.findIntersection(solid_box))
        {
            if (velocity_y > 0.f)   // falling: land on top of the solid
            {
                shape.setPosition({ player_box.position.x, solid_box.position.y - player_box.size.y });
                velocity_y = 0.f;
            }
            else if (velocity_y < 0.f)   // going up: bump the head on the underside
            {
                shape.setPosition({ player_box.position.x, solid_box.position.y + solid_box.size.y });
                velocity_y = 0.f;
            }
        }
    }

    // ---- Ground check ----
    // Imagine the hitbox moved 1 pixel down: if that touches a solid, we are standing on it
    is_on_ground = false;
    if (velocity_y >= 0.f)
    {
        sf::FloatRect probe(
            { shape.getPosition().x, shape.getPosition().y + 1.f },
            shape.getSize()
        );
        for (const auto& solid : solids)
        {
            if (probe.findIntersection(solid.getGlobalBounds()))
            {
                is_on_ground = true;
                break;
            }
        }
    }

    if (is_on_ground)
        is_clinging = false;

    if (is_on_ground && !is_dashing)
        can_dash = true;   // touching the ground gives the dash back

    // ---- World boundaries ----
    if (shape.getPosition().x < 0.f)
        shape.setPosition({ 0.f, shape.getPosition().y });
    if (shape.getPosition().x + shape.getSize().x > world_width)
        shape.setPosition({ world_width - shape.getSize().x, shape.getPosition().y });

    // ---- Animation: pick which picture from the sheet to show ----
    anim_timer += delta_time;

    int row = 0;   // which animation (row of the sheet)
    int col = 0;   // which picture of that animation (column)

    if (is_striking)
    {
        row = 3;                                                   // sword attack
        col = static_cast<int>(strike_timer / strike_duration * 10.f);
        if (col > 9) col = 9;
    }
    else if (is_dashing)
    {
        row = 2;                                                   // dash = one running picture
        col = 4;
    }
    else if (!is_on_ground)
    {
        row = 6;                                                   // in the air
        if (velocity_y < -100.f)     col = 1;                       // going up
        else if (velocity_y > 100.f) col = 5;                       // falling
        else                        col = 3;                       // top of the jump
    }
    else if (is_crouching)
    {
        row = 6;                                                   // crouch
        col = 8;
    }
    else if (input_dir != 0.f)
    {
        row = 2;                                                   // running
        col = static_cast<int>(anim_timer * 14.f) % 10;
    }
    else
    {
        row = 0;                                                   // idle
        col = static_cast<int>(anim_timer * 6.f) % 5;
    }

    sprite.setTextureRect(sf::IntRect({ col * 60, row * 56 }, { 60, 56 }));

    // Died: respawn with full health
    if (health <= 0)
    {
        shape.setPosition({ 100.f, 100.f });
        velocity_y = 0.f;
        health = max_health;
        invincible_timer = 1.5f;
        is_clinging = false;
    }
}

void Player::draw(sf::RenderWindow& window)
{
    // While invincible the knight blinks (hidden every other 0.1 seconds)
    bool blink_off = invincible_timer > 0.f && int(invincible_timer * 10.f) % 2 == 0;

    if (!blink_off)
    {
        sf::FloatRect body_box = shape.getGlobalBounds();

        sprite.setOrigin({ 30.f, 54.f });  // the knight's feet
        sprite.setScale({ sprite_scale * facing_direction, sprite_scale });
        // Feet at the bottom middle of the hitbox
        sprite.setPosition({ body_box.position.x + body_box.size.x / 2.f,
                             body_box.position.y + body_box.size.y });
        window.draw(sprite);
    }

    // The sword's hitbox, shown in red while it can hit (debug view)
    if (isStrikeActive())
    {
        sf::FloatRect sword_box = getStrikeHitbox();
        sf::RectangleShape debug_strike(sword_box.size);
        debug_strike.setPosition(sword_box.position);
        debug_strike.setFillColor(sf::Color(255, 0, 0, 120));
        window.draw(debug_strike);
    }

    // Set debug_hitbox = true in Player.h to see the knight's hitbox
    if (debug_hitbox)
    {
        sf::RectangleShape debug_box(shape.getSize());

        debug_box.setPosition(shape.getPosition());
        debug_box.setFillColor(sf::Color(0, 255, 0, 60));
        debug_box.setOutlineColor(sf::Color::Green);
        debug_box.setOutlineThickness(2.f);

        window.draw(debug_box);

        // Yellow = active sword hitbox
        if (isStrikeActive())
        {
            sf::FloatRect strike_box = getStrikeHitbox();

            sf::RectangleShape sword_box(strike_box.size);
            sword_box.setPosition(strike_box.position);

            sword_box.setFillColor(sf::Color(255, 255, 0, 60));
            sword_box.setOutlineColor(sf::Color::Yellow);
            sword_box.setOutlineThickness(2.f);

            window.draw(sword_box);
        }
    }
}

// The middle of the knight's hitbox
sf::Vector2f Player::getCenter() const
{
    return {
        shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f
    };
}

// The sword only hurts during the middle part of the swing
bool Player::isStrikeActive() const
{
    return is_striking
        && strike_timer >= strike_active_start
        && strike_timer <= strike_active_end;
}

// A box in front of the knight, on the side he is facing
sf::FloatRect Player::getStrikeHitbox() const
{
    float x;
    if (facing_direction > 0.f)
        x = shape.getPosition().x + shape.getSize().x;   // right edge
    else
        x = shape.getPosition().x - strike_range;         // left of player

    float y = shape.getPosition().y + shape.getSize().y * 0.25f;

    return sf::FloatRect({ x, y }, { strike_range, strike_height });
}

int Player::getStrikeId() const
{
    return strike_id;
}

void Player::takeDamage(int amount, float from_direction)
{
    if (invincible_timer > 0.f)
        return;   // can't be hurt again right after a hit

    health -= amount;
    invincible_timer = invincible_time;

    // Throw the knight back and a little up
    knockback_timer = 0.2f;
    knockback_push_x = from_direction * 300.f;
    velocity_y = -250.f;
    is_on_ground = false;

    // Getting hit cancels whatever we were doing
    is_dashing = false;
    is_striking = false;
    is_clinging = false;
}

sf::FloatRect Player::getBounds() const { return shape.getGlobalBounds(); }
float Player::getFacing() const { return facing_direction; }
int Player::getHealth() const { return health; }
int Player::getMaxHealth() const { return max_health; }

// Checks for a tall wall touching the left or right side of the knight
// Returns -1 (wall on the left), 1 (wall on the right) or 0 (no wall)
float Player::detectWall(const std::vector<sf::RectangleShape>& solids) const
{
    sf::FloatRect body_box = shape.getGlobalBounds();
    // Two thin strips, one just outside each side of the knight
    sf::FloatRect left_probe({ body_box.position.x - 2.f, body_box.position.y }, { 2.f, body_box.size.y });
    sf::FloatRect right_probe({ body_box.position.x + body_box.size.x, body_box.position.y }, { 2.f, body_box.size.y });

    for (const auto& solid : solids)
    {
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (solid_box.size.y < min_wall_height)
            continue;   // too short to be a wall (it is a platform)
        if (left_probe.findIntersection(solid_box))  return -1.f;
        if (right_probe.findIntersection(solid_box)) return 1.f;
    }
    return 0.f;
}

bool Player::canStandUp(const std::vector<sf::RectangleShape>& solids) const
{
    // Imagine the tall standing box with feet in the same place
    float bottom = shape.getPosition().y + shape.getSize().y;
    sf::FloatRect standing_box(
        { shape.getPosition().x, bottom - stand_height },
        { hitbox_width, stand_height - 1.f }
    );

    for (const auto& solid : solids)
    {
        if (standing_box.findIntersection(solid.getGlobalBounds()))
            return false; // something is above our head
    }
    return true;
}

// Put the knight back at the start with full health (used when a new game begins)
void Player::reset()
{
    shape.setSize({ hitbox_width, stand_height });
    shape.setPosition({ 100.f, 200.f });
    velocity_y = 0.f;
    health = max_health;
    invincible_timer = 0.f;
    knockback_timer = 0.f;
    is_dashing = is_striking = is_clinging = is_crouching = false;
    can_dash = true;
}
