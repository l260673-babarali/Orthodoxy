#include "../Classes/Player.h"
#include<iostream>


Player::Player() : sprite(sheet_texture)
{
    shape.setSize({ 50.f, 80.f });

    // Spawn on the monastery's ground
    shape.setPosition({ 100.f, 200.f });

    if (!sheet_texture.loadFromFile(
        "Assets/Sprites/Player/ashen_knight_atlas.png"))
    {
        std::cout << "Could not load ashen_knight_atlas.png\n";
    }

    sprite.setTexture(sheet_texture);
    sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 192, 192 }));
}


// Runs every frame: reads the keys, moves the knight, handles collisions and picks the animation
void Player::update(
    float delta_time, const std::vector<sf::RectangleShape>& solids,
    float world_width, float world_height
)
{
    // Base speed 200 hai; har extra Agility level par 12 add karte hain. Agility 1 par bonus zero hota hai.
    const float speed = 200.f + (agility - 1) * 12.f;
    const float gravity = 981.f;
    // SFML mein negative Y upar hota hai, is liye jump velocity negative hai.
    const float jump_velocity = -550.f;         // negative = up
    // Double jump ki upward speed primary jump se thori kam rakhi hai (-480 vs -550).
    const float double_jump_velocity = -480.f;

    // ---- Count the timers down ----
    if (invincible_timer > 0.f)     invincible_timer -= delta_time;
    if (dash_cooldown_timer > 0.f)   dash_cooldown_timer -= delta_time;
    if (strike_cooldown_timer > 0.f) strike_cooldown_timer -= delta_time;
    if (heavy_cooldown_timer > 0.f) heavy_cooldown_timer -= delta_time;

    if (is_striking)
    {
        strike_timer += delta_time;
        if (strike_timer >= strike_duration)
            is_striking = false;
    }

    if (is_heavy_striking)
    {
        heavy_timer += delta_time;
        if (heavy_timer >= heavy_duration)
            is_heavy_striking = false;
    }

    // ---- Raw input (edge-detected) ----
    // "Pressed" = went down this frame. "Held" = is down right now.
    bool strike_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J);
    bool strike_pressed = strike_held && !strike_was_held;
    strike_was_held = strike_held;

    bool jump_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    bool jump_pressed = jump_held && !jump_was_held;
    jump_was_held = jump_held;

    bool heavy_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K);
    bool heavy_pressed = heavy_held && !heavy_was_held;
    heavy_was_held = heavy_held;

    if (heavy_charging)
    {
        if (!heavy_held)
        {
            // K jaldi chhorne par charge cancel ho jata hai.
            heavy_charging = false;
            heavy_charge_timer = 0.f;
        }
        else
        {
            heavy_charge_timer += delta_time;
            if (heavy_charge_timer >= heavy_charge_time)
            {
                // K ko 2 seconds hold karne ke baad heavy swing hota hai.
                heavy_charging = false;
                heavy_charge_timer = 0.f;
                is_heavy_striking = true;
                heavy_timer = 0.f;
                heavy_cooldown_timer = heavy_cooldown;
                strike_id++;
            }
        }
    }

    float input_dir = 0.f;   // -1 = left, 0 = nothing, 1 = right
    // A press karne se horizontal direction -1 yani left banti hai.
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) input_dir -= 1.f;
    // D press karne se direction +1 yani right banti hai; A aur D dono par 0 ho sakti hai.
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
    if (strike_pressed && !is_striking && !is_heavy_striking && !heavy_charging && !is_dashing && !near_wall && strike_cooldown_timer <= 0.f)
    {
        is_striking = true;
        strike_timer = 0.f;
        strike_cooldown_timer = strike_cooldown;
        strike_id++;
    }

    if (heavy_pressed && !heavy_charging && !is_heavy_striking && !is_striking
        && !is_dashing && !near_wall && heavy_cooldown_timer <= 0.f)
    {
        heavy_charging = true;
        heavy_charge_timer = 0.f;
    }

    // ---- Crouch ----
    bool crouch_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    bool want_crouch = crouch_held && is_on_ground && !is_clinging;

    if (want_crouch && !is_crouching)
    {
        // Shrink the box, but keep the feet in the same spot
        // Character ke feet/bottom Y = top Y + hitbox height.
        float bottom = shape.getPosition().y + shape.getSize().y;
        shape.setSize({ hitbox_width, crouch_height });
        // Crouch par new top = old bottom - new crouch height; feet ki position same rehti hai.
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
            // Khara hone par new top = old bottom - standing height; feet floor par same rehte hain.
            // Stand check mein proposed head Y = feet - standing height; head ko ceiling se test karte hain.
            shape.setPosition({ shape.getPosition().x, bottom - stand_height });
            is_crouching = false;
        }
    }

    // ---- Horizontal input ----
    // move_x = how many pixels to move sideways this frame
    float move_x = 0.f;
    if (input_dir < 0.f)
    {
        // Left movement distance = speed * seconds; subtract se left jata hai, FPS par depend nahi karta.
        move_x -= speed * delta_time;
        if (!is_clinging) facing_direction = -1.f;
    }
    if (input_dir > 0.f)
    {
        // Right movement distance = speed * frame time; har frame ki time se consistent speed milti hai.
        move_x += speed * delta_time;
        if (!is_clinging) facing_direction = 1.f;
    }

    if (is_striking && is_on_ground)
        move_x *= 0.3f;

    if ((is_heavy_striking || heavy_charging) && is_on_ground)
        move_x *= 0.15f;

    if (is_crouching)
        // Crouch movement = normal movement x crouch multiplier, is se movement slow hoti hai.
        move_x *= crouch_speed_multiplier;

    // Being pushed back after getting hit
    if (knockback_timer > 0.f)
    {
        // Knockback speed ko frame time se multiply kar ke is frame ka extra horizontal push milta hai.
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
        // Coyote timer har frame guzre hue seconds se kam hota hai; chhoti grace period deta hai.
        coyote_timer -= delta_time;
    }

    // ---- Drop through a thin platform: S + Space while standing on it ----
    bool drop_started = false;
    if (jump_pressed && crouch_held && is_on_ground && !is_dashing && !near_wall && drop_timer <= 0.f)
    {
        std::vector<sf::FloatRect> standing_on;
        if (findDropPlatforms(solids, world_height, standing_on))
        {
            drop_ignored = standing_on;
            drop_timer = drop_time;
            velocity_y = drop_start_speed;   // start falling right away
            is_on_ground = false;
            coyote_timer = 0.f;
            drop_started = true;             // this Space press was the drop, not a jump
        }
    }
    if (drop_timer > 0.f)
    {
        drop_timer -= delta_time;
        if (drop_timer <= 0.f)
            drop_ignored.clear();            // the platform is solid again
    }

    if (jump_pressed && !is_dashing && !near_wall && !drop_started)
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
        && can_dash && !is_dashing && !is_striking && !is_heavy_striking && !heavy_charging && !is_clinging && dash_cooldown_timer <= 0.f)
    {
        is_dashing = true;
        can_dash = false;
        dash_timer = dash_duration;
        // Dash cooldown = base cooldown - (Agility extra levels x 0.065 seconds).
        dash_cooldown_timer = dash_cooldown - (agility - 1) * 0.065f;
        velocity_y = 0.f;
    }

    if (is_dashing)
    {
        // During a dash we fly straight, with no gravity
        // Dash distance = direction (+1/-1) x dash speed x frame seconds.
        move_x = facing_direction * dash_speed * delta_time;
        velocity_y = 0.f;
        dash_timer -= delta_time;
        if (dash_timer <= 0.f)
            is_dashing = false;
    }
    else
    {
        // Gravity acceleration ko seconds se multiply kar ke vertical velocity mein add karte hain: v = v + g*dt.
        velocity_y += gravity * delta_time;
        // Clinging: gravity still pulls us down, but only as a slow slide
        if (is_clinging && velocity_y > wall_slide_speed)
            velocity_y = wall_slide_speed;
    }

    // ---- Horizontal movement + collision ----
    // Move first, then fix it: if we ended up inside a solid, push back out
    bool grounded_before_move = is_on_ground;   // used for walking up / down ramps
    shape.move({ move_x, 0.f });
    if (move_x != 0.f)
    {
        for (const auto& solid : solids)
        {
            sf::FloatRect player_box = shape.getGlobalBounds();
            sf::FloatRect solid_box = solid.getGlobalBounds();
            if (isDropIgnored(solid_box))
                continue;   // we are falling through this platform
            if (player_box.findIntersection(solid_box))
            {
                // A small step (a ramp column): climb onto it instead of stopping
                float step_height = (player_box.position.y + player_box.size.y) - solid_box.position.y;
                if (grounded_before_move && velocity_y >= 0.f && step_height > 0.f && step_height <= max_step_up)
                {
                    sf::FloatRect lifted({ player_box.position.x, solid_box.position.y - player_box.size.y }, player_box.size);
                    bool head_free = true;
                    for (const auto& other : solids)
                    {
                        if (isDropIgnored(other.getGlobalBounds()))
                            continue;
                        if (lifted.findIntersection(other.getGlobalBounds()))
                        {
                            head_free = false;
                            break;
                        }
                    }
                    if (head_free)
                    {
                        shape.setPosition({ lifted.position.x, lifted.position.y });
                        continue;
                    }
                }
                if (move_x > 0.f)   // moving right: stop at the solid's left side
                    // Right ja kar wall se takraye to player ka left X = wall ka left X - player width.
                    shape.setPosition({ solid_box.position.x - player_box.size.x, player_box.position.y });
                else               // moving left: stop at the solid's right side
                    // Left ja kar wall se takraye to player ka X = wall ka right edge.
                    shape.setPosition({ solid_box.position.x + solid_box.size.x, player_box.position.y });
                is_dashing = false;
            }
        }
    }

    // ---- Vertical movement + collision ----
    // Vertical distance = vertical speed x frame time; gravity ne velocity pehle update ki hoti hai.
    shape.move({ 0.f, velocity_y * delta_time });
    for (const auto& solid : solids)
    {
        sf::FloatRect player_box = shape.getGlobalBounds();
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (isDropIgnored(solid_box))
            continue;   // we are falling through this platform
        if (player_box.findIntersection(solid_box))
        {
            if (velocity_y > 0.f)   // falling: land on top of the solid
            {
                // Neeche girte hue player ka top = platform top - player height; feet platform par aa jate hain.
                shape.setPosition({ player_box.position.x, solid_box.position.y - player_box.size.y });
                velocity_y = 0.f;
            }
            else if (velocity_y < 0.f)   // going up: bump the head on the underside
            {
                // Ooper jump par collision ho to player ka top = ceiling ka bottom, taake overlap na ho.
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
            if (isDropIgnored(solid.getGlobalBounds()))
                continue;   // not standing on a platform we are dropping through
            if (probe.findIntersection(solid.getGlobalBounds()))
            {
                is_on_ground = true;
                break;
            }
        }
    }

    // Walking down a ramp: if the ground is only a few pixels below, stay on it
    if (!is_on_ground && grounded_before_move && velocity_y >= 0.f && move_x != 0.f)
    {
        for (int drop = 2; drop <= static_cast<int>(max_step_down); drop++)
        {
            sf::FloatRect probe(
                { shape.getPosition().x, shape.getPosition().y + static_cast<float>(drop) },
                shape.getSize()
            );
            bool found = false;
            for (const auto& solid : solids)
            {
                if (isDropIgnored(solid.getGlobalBounds()))
                    continue;
                if (probe.findIntersection(solid.getGlobalBounds()))
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                shape.move({ 0.f, static_cast<float>(drop - 1) });
                velocity_y = 0.f;
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
    // Right boundary = player left X + width; agar world width se zyada ho to wapas andar karo.
    if (shape.getPosition().x + shape.getSize().x > world_width)
        // Player ka maximum left X = world width - player width; poora hitbox andar rehta hai.
        shape.setPosition({ world_width - shape.getSize().x, shape.getPosition().y });

    int row = 0;
    int col = 0;

    if (is_heavy_striking || heavy_charging)
    {
        row = 8;
    }
    else if (is_striking)
    {
        row = 7;
    }
    else if (is_dashing)
    {
        row = 6;
    }
    else if (!is_on_ground)
    {
        row = 5;
    }
    else if (is_crouching)
    {
        if (input_dir == 0.f)
            row = 3;
        else
            row = 4;
    }
    else if (input_dir != 0.f)
    {
        row = 1;
    }

    if (row != last_animation_row)
    {
        anim_timer = 0.f;
        last_animation_row = row;
    }

    anim_timer += delta_time;

    switch (row)
    {
    case 0:
        col = int(anim_timer * 5.f) % 8;
        break;
    case 1:
        col = int(anim_timer * 8.f) % 8;
        break;
    case 2:
        col = int(anim_timer * 9.f) % 8;
        break;
    case 3:
        col = int(anim_timer * 5.f) % 4;
        break;
    case 4:
        col = int(anim_timer * 7.f) % 8;
        break;
    case 5:
        if (velocity_y < -320.f)
            col = 0;
        else if (velocity_y < -120.f)
            col = 1;
        else if (velocity_y < 60.f)
            col = 2;
        else if (velocity_y < 220.f)
            col = 3;
        else if (velocity_y < 380.f)
            col = 4;
        else
            col = 5;
        break;
    case 6:
        col = int(anim_timer * 14.f) % 6;
        break;
    case 7:
        col = int(strike_timer / strike_duration * 6.f);
        if (col > 5)
            col = 5;
        break;
    case 8:
        if (heavy_charging)
        {
            // Charge ke duran wind-up ke pehle 4 frames dikhte hain.
            col = int(heavy_charge_timer / heavy_charge_time * 4.f);
            if (col > 3) col = 3;
        }
        else
        {
            // Swing charge ke baad ke 5 frames mein chalta hai.
            col = 4 + int(heavy_timer / heavy_duration * 5.f);
            if (col > 8) col = 8;
        }
        break;
    }

    sprite.setTextureRect(sf::IntRect({ col * 192, row * 192 }, { 192, 192 }));

    // Health zero par Engine SOUL FORFEITED screen dikhayega.
    // Automatic respawn nahi hoga; Retry se level dobara shuru hota hai.
}

void Player::draw(sf::RenderWindow& window)
{
    // While invincible the knight blinks (hidden every other 0.1 seconds)
    // Invincibility blink: timer x10 ko integer kar ke %2 se alternate visible/hidden frames milte hain.
    bool blink_off = invincible_timer > 0.f && int(invincible_timer * 10.f) % 2 == 0;

    if (!blink_off)
    {
        sf::FloatRect body_box = shape.getGlobalBounds();

        sprite.setOrigin({ 96.f, 169.f });
        // Sprite ka horizontal scale facing +/-1 se multiply hota hai; negative sign image ko left flip karta hai.
        sprite.setScale({ sprite_scale * facing_direction, sprite_scale });
        // Feet at the bottom middle of the hitbox
        // Sprite center X = player hitbox left + half width; feet Y = hitbox top + height.
        // Right probe ka X = player ke right edge par hota hai.
        sprite.setPosition({ body_box.position.x + body_box.size.x / 2.f,
                             body_box.position.y + body_box.size.y });
        // See-through while dashing so you can tell the dodge is active
        sprite.setColor(is_dashing ? sf::Color(255, 255, 255, 120) : sf::Color::White);
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
        // Hitbox center X = left X + width/2; center Y = top Y + height/2.
        shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f
    };
}

// The sword only hurts during the middle part of the swing
bool Player::isStrikeActive() const
{
    if (is_heavy_striking)
    {
        return heavy_timer >= heavy_active_start
            && heavy_timer <= heavy_active_end;
    }

    return is_striking
        && strike_timer >= strike_active_start
        && strike_timer <= strike_active_end;
}

// A box in front of the knight, on the side he is facing
sf::FloatRect Player::getStrikeHitbox() const
{
    float used_range = strike_range;
    float used_height = strike_height;

    if (is_heavy_striking)
    {
        used_range = heavy_range;
        used_height = heavy_height;
    }

    float x;
    if (facing_direction > 0.f)
        x = shape.getPosition().x + shape.getSize().x;
    else
        x = shape.getPosition().x - used_range;

    float y = shape.getPosition().y + shape.getSize().y * 0.20f;

    return sf::FloatRect({ x, y }, { used_range, used_height });
}

int Player::getStrikeId() const
{
    return strike_id;
}

void Player::takeDamage(int amount, float from_direction)
{
    if (is_dashing)
        return;   // DODGE: attacks pass through the knight while he dashes

    if (invincible_timer > 0.f)
        return;   // can't be hurt again right after a hit

    hits_taken++;
    if (endurance > 1)
    {
        // Endurance barhne par blocks ke darmiyan hits kam hote hain: 7 - Endurance.
        int hits_between_blocks = 7 - endurance;
        // Remainder % zero ho to har Nth hit par 1 damage block hota hai.
        if (hits_taken % hits_between_blocks == 0)
        {
            amount = amount - 1;
        }
    }
    if (amount <= 0)
    {
        invincible_timer = invincible_time;
        return;
    }

    health -= amount;
    invincible_timer = invincible_time;

    // Throw the knight back and a little up
    knockback_timer = 0.2f;
    // Hit ki direction +/-1 ko push speed 300 se multiply karte hain taake character door jaye.
    knockback_push_x = from_direction * 300.f;
    velocity_y = -250.f;
    is_on_ground = false;

    // Getting hit cancels whatever we were doing
    is_dashing = false;
    is_striking = false;
    is_heavy_striking = false;
    heavy_charging = false;
    heavy_charge_timer = 0.f;
    is_clinging = false;
}

sf::FloatRect Player::getBounds() const { return shape.getGlobalBounds(); }
float Player::getFacing() const { return facing_direction; }
int Player::getHealth() const { return health; }
bool Player::isDodging() const { return is_dashing; }
int Player::getMaxHealth() const { return max_health; }

int Player::getStat(int choice) const
{
    switch (choice)
    {
    case 0:
        return vitality;
    case 1:
        return strength;
    case 2:
        return endurance;
    case 3:
        return agility;
    default:
        return 0;
    }
}

float Player::getHeavyCharge() const
{
    if (!heavy_charging) return 0.f;
    return heavy_charge_timer / heavy_charge_time;
}

void Player::restoreProgress(float x, float y, int savedHealth,
    int savedVitality, int savedStrength, int savedEndurance, int savedAgility)
{
    vitality = savedVitality;
    strength = savedStrength;
    endurance = savedEndurance;
    agility = savedAgility;
    max_health = 4 + vitality;
    reset({ x, y });
    health = savedHealth;
    if (health > max_health) health = max_health;
    if (health < 1) health = 1;
}

int Player::getSwordDamage() const
{
    if (is_heavy_striking)
        return strength + 1;

    return strength;
}

bool Player::upgradeStat(int choice)
{
    if (choice < 0 || choice > 3)
    {
        return false;
    }

    if (getStat(choice) >= 5)
    {
        return false;
    }

    switch (choice)
    {
    case 0:
        vitality++;
        max_health++;
        health++;
        break;
    case 1:
        strength++;
        break;
    case 2:
        endurance++;
        break;
    case 3:
        agility++;
        break;
    default:
        return false;
    }

    return true;
}

void Player::resetStats()
{
    vitality = 1;
    strength = 1;
    endurance = 1;
    agility = 1;
    max_health = 5;
    health = max_health;
    hits_taken = 0;
}

// Checks for a tall wall touching the left or right side of the knight
// Returns -1 (wall on the left), 1 (wall on the right) or 0 (no wall)
float Player::detectWall(const std::vector<sf::RectangleShape>& solids) const
{
    sf::FloatRect body_box = shape.getGlobalBounds();
    // Two thin strips, one just outside each side of the knight
    // Left wall probe ko player ke left se 2 pixels bahar rakhte hain.
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

// Is this solid one of the platforms we are dropping through right now?
bool Player::isDropIgnored(const sf::FloatRect& solid_box) const
{
    for (const auto& ignored : drop_ignored)
    {
        if (ignored == solid_box)
            return true;
    }
    return false;
}

// Can we drop through what we stand on? Fills 'standing_on' with the platform(s) under the feet.
// It only works on thin platforms, and only if there is something below to land on
// (so the knight never drops out of the level).
bool Player::findDropPlatforms(const std::vector<sf::RectangleShape>& solids, float world_height,
    std::vector<sf::FloatRect>& standing_on) const
{
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f size = shape.getSize();
    float feet_y = pos.y + size.y;
    sf::FloatRect feet_probe({ pos.x, pos.y + 1.f }, size);

    for (const auto& solid : solids)
    {
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (!feet_probe.findIntersection(solid_box))
            continue;
        if (solid_box.size.y > thin_platform_height)
            return false;   // a real floor, not a platform
        standing_on.push_back(solid_box);
    }
    if (standing_on.empty())
        return false;

    // Look for a landing spot below: lower than the feet, under the knight, and not at the very bottom of the level
    for (const auto& solid : solids)
    {
        sf::FloatRect solid_box = solid.getGlobalBounds();
        bool is_current = false;
        for (const auto& current : standing_on)
        {
            if (current == solid_box)
                is_current = true;
        }
        if (is_current)
            continue;
        bool below = solid_box.position.y > feet_y + 20.f;
        bool under_knight = solid_box.position.x < pos.x + size.x
            && solid_box.position.x + solid_box.size.x > pos.x;
        bool not_the_void_floor = solid_box.position.y < world_height - 40.f;   // the safety floor at the very bottom does not count
        if (below && under_knight && not_the_void_floor)
            return true;
    }
    return false;
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
void Player::reset(sf::Vector2f spawn)
{
    spawn_point = spawn;
    shape.setSize({ hitbox_width, stand_height });
    shape.setPosition(spawn);
    velocity_y = 0.f;
    health = max_health;
    invincible_timer = 0.f;
    knockback_timer = 0.f;
    is_dashing = false;
    is_striking = false;
    is_heavy_striking = false;
    heavy_charging = false;
    heavy_charge_timer = 0.f;
    heavy_was_held = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K);
    is_clinging = false;
    is_crouching = false;
    can_dash = true;
    drop_timer = 0.f;
    drop_ignored.clear();
}



