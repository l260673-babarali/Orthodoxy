#include "../Classes/Enemy.h"

Enemy::Enemy(sf::Vector2f position, sf::Vector2f size, sf::Color color, int start_health)
    : base_color(color), health(start_health)
{
    max_health = start_health;
    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(color);
}

// Turn white for a moment after being hit
void Enemy::updateHurtFlash(float delta_time)
{
    if (hurt_timer > 0.f)
    {
        hurt_timer -= delta_time;
        shape.setFillColor(sf::Color::White);
    }
    else
    {
        shape.setFillColor(base_color);
    }
}

// Returns how far to push the enemy this frame after being hit (0 if not hit)
float Enemy::consumeKnockback(float delta_time)
{
    if (knockback_timer <= 0.f)
        return 0.f;

    knockback_timer -= delta_time;
    // Enemy ko hit par push distance = direction x knockback speed x frame time.
    return knockback_dir * knockback_speed * delta_time;
}

// Walk sideways and stop at walls. Returns true if we hit a wall.
bool Enemy::moveHorizontal(float move_x, const std::vector<sf::RectangleShape>& solids)
{
    if (move_x == 0.f)
        return false;

    bool hit_wall = false;
    shape.move({ move_x, 0.f });

    // If we ended up inside a solid, push ourselves back out on the side we came from
    for (const auto& solid : solids)
    {
        sf::FloatRect enemy_box = shape.getGlobalBounds();
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (enemy_box.findIntersection(solid_box))
        {
            // Thin platforms and ramps only hold you up from above: a small step is climbed,
            // anything else is passed through
            bool thin = solid_box.size.y < 20.f;
            float step_height = (enemy_box.position.y + enemy_box.size.y) - solid_box.position.y;
            if (velocity_y == 0.f && step_height > 0.f && step_height <= 16.f)
            {
                sf::FloatRect lifted({ enemy_box.position.x, solid_box.position.y - enemy_box.size.y }, enemy_box.size);
                bool head_free = true;
                for (const auto& other : solids)
                    if (other.getGlobalBounds().size.y >= 20.f && lifted.findIntersection(other.getGlobalBounds()))
                        head_free = false;
                if (head_free)
                {
                    shape.setPosition({ lifted.position.x, lifted.position.y });
                    continue;
                }
            }
            if (thin)
                continue;
            if (move_x > 0.f)   // moving right: stop at the solid's left side
                // Enemy right wall par aaye to uska X = wall start - enemy width.
                shape.setPosition({ solid_box.position.x - enemy_box.size.x, enemy_box.position.y });
            else               // moving left: stop at the solid's right side
                // Enemy left wall par aaye to uska X = wall right edge.
                shape.setPosition({ solid_box.position.x + solid_box.size.x, enemy_box.position.y });
            hit_wall = true;
        }
    }
    return hit_wall;
}

// Fall down and stand on the floor
void Enemy::applyGravity(float delta_time, const std::vector<sf::RectangleShape>& solids)
{
    const float gravity = 981.f;

    // Enemy ki Y velocity har frame gravity x seconds se barhti hai.
    velocity_y += gravity * delta_time;
    float feet_before = shape.getPosition().y + shape.getSize().y;
    // Enemy vertical movement = updated vertical velocity x seconds.
    shape.move({ 0.f, velocity_y * delta_time });

    for (const auto& solid : solids)
    {
        sf::FloatRect enemy_box = shape.getGlobalBounds();
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (enemy_box.findIntersection(solid_box))
        {
            bool thin = solid_box.size.y < 20.f;
            if (velocity_y > 0.f)   // falling: land on top of the solid
            {
                if (thin && feet_before > solid_box.position.y + 0.5f)
                    continue;   // we are below this thin platform: it does not catch us
                // Landing position = platform top - enemy height, taake enemy floor par khara ho.
                shape.setPosition({ enemy_box.position.x, solid_box.position.y - enemy_box.size.y });
                velocity_y = 0.f;
            }
            else if (velocity_y < 0.f)   // going up: bump the head on the underside
            {
                if (thin)
                    continue;
                // Ceiling se takra kar enemy ka top ceiling ke bottom par set karte hain.
                shape.setPosition({ enemy_box.position.x, solid_box.position.y + solid_box.size.y });
                velocity_y = 0.f;
            }
        }
    }
}

void Enemy::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;

    window.draw(shape);

    // Blue = enemy body hitbox
    sf::RectangleShape hurtbox(shape.getSize());

    hurtbox.setPosition(shape.getPosition());
    hurtbox.setFillColor(sf::Color::Transparent);
    hurtbox.setOutlineColor(sf::Color::Cyan);
    hurtbox.setOutlineThickness(5.f);

    window.draw(hurtbox);

    // Red = active enemy attack hitbox
    sf::FloatRect box;
    if (getAttackHitbox(box))
    {
        sf::RectangleShape attack_box(box.size);
        attack_box.setPosition(box.position);
        attack_box.setFillColor(sf::Color(255, 0, 0, 100));

        window.draw(attack_box);
    }
}

// Set this to true to see the red attack boxes again (useful while tuning attacks)
namespace
{
    constexpr bool show_attack_boxes = false;
}

void Enemy::drawAttackBox(sf::RenderWindow& window) const
{
    if (!show_attack_boxes)
        return;
    sf::FloatRect box;
    if (getAttackHitbox(box))
    {
        sf::RectangleShape attack_box(box.size);
        attack_box.setPosition(box.position);
        attack_box.setFillColor(sf::Color(255, 0, 0, 100));
        window.draw(attack_box);
    }
}

void Enemy::takeDamage(int amount, int strike_id, float from_direction)
{
    health -= amount;
    last_hit_strike_id = strike_id;
    hurt_timer = 0.15f;
    knockback_timer = 0.15f;
    knockback_dir = from_direction;

    if (health <= 0)
    {
        defeated_by_player = true;
        alive = false;
    }
}

void Enemy::setDifficulty(int level)
{
    difficulty_level = level;
    health += (level - 1) * 2;
    max_health = health;
}

int Enemy::getAttackDamage() const
{
    if (difficulty_level == 3)
        return 2;
    return 1;
}

float Enemy::getHealthFraction() const
{
    if (max_health <= 0 || health <= 0)
        return 0.f;
    // Health bar fill = current health / health at the start; never above 1
    float fraction = static_cast<float>(health) / max_health;
    if (fraction > 1.f)
        fraction = 1.f;
    return fraction;
}

bool Enemy::isAlive() const { return alive; }
bool Enemy::wasDefeatedByPlayer() const { return defeated_by_player; }
int Enemy::getLastHitStrikeId() const { return last_hit_strike_id; }
sf::FloatRect Enemy::getBounds() const { return shape.getGlobalBounds(); }

// The middle of the enemy's body
sf::Vector2f Enemy::getCenter() const
{
    return {
        shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f
    };
}

// Golem ke functions yahan hain.
#include <iostream>

Golem::Golem(float x, float y)
    : Enemy({ x, y }, { 80.f, 110.f }, sf::Color(110, 110, 120), 6), sprite(texture)
{
    knockback_speed = 60.f; // heavy, barely gets pushed

    if (!texture.loadFromFile("Assets/Sprites/Enemies/Golem/golem_sheet.png"))
        std::cout << "Could not load golem_sheet.png\n";
    sprite.setTexture(texture, true);
    updateAnimation();   // show the first frame at once (otherwise the whole sprite sheet is drawn before the first update)
}

void Golem::update(float delta_time, const std::vector<sf::RectangleShape>& solids,
    sf::Vector2f player_center, EnemyWorld& world)
{
    if (!alive)
        return;

    updateHurtFlash(delta_time);
    anim_timer += delta_time;

    if (throw_cooldown > 0.f) throw_cooldown -= delta_time;
    if (smash_cooldown > 0.f) smash_cooldown -= delta_time;
    state_timer += delta_time;

    moveHorizontal(consumeKnockback(delta_time), solids);

    // x_to_player is positive when the player is on our right, negative when on our left
    // Player tak horizontal gap = player X - enemy X; sign se left/right direction pata chalti hai.
    float x_to_player = player_center.x - getCenter().x;
    float distance_to_player = std::abs(x_to_player);

    // The golem works like a small state machine:
    // Idle/Walk -> ThrowWindup or SmashWindup -> Recover -> Idle
    switch (state)
    {
    case State::Idle:
    case State::Walk:
        if (distance_to_player > sight_range)
        {
            state = State::Idle;
            break;
        }
        facing = (x_to_player >= 0.f) ? 1.f : -1.f;

        if (distance_to_player <= smash_range && smash_cooldown <= 0.f)
        {
            // Player is close: smash the ground
            state = State::SmashWindup;
            state_timer = 0.f;
        }
        else if (distance_to_player >= throw_min_range && throw_cooldown <= 0.f)
        {
            // Player is far: throw a rock
            state = State::ThrowWindup;
            state_timer = 0.f;
        }
        else if (distance_to_player > smash_range * 0.6f)
        {
            // Slowly walk toward the player
            state = State::Walk;
            // Enemy walk distance = facing sign x speed x frame time.
            moveHorizontal(facing * walk_speed * delta_time * (1.f + 0.15f * (difficulty_level - 1)), solids);
        }
        break;

    case State::ThrowWindup:
        facing = (x_to_player >= 0.f) ? 1.f : -1.f;
        if (state_timer >= 0.6f)
        {
            throwRock(player_center, world);
            throw_cooldown = 2.5f;
            last_attack_was_smash = false;
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    case State::SmashWindup:
        if (state_timer >= 0.8f)
        {
            smashGround(world);
            smash_cooldown = 3.f;
            last_attack_was_smash = true;
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    case State::Recover:
        if (state_timer >= 0.6f)
        {
            state = State::Idle;
            state_timer = 0.f;
        }
        break;
    }

    applyGravity(delta_time, solids);
    updateAnimation();
}

// Pick which picture of golem_sheet.png to show.
// Rows: 0 = idle (10), 1 = walk (10), 2 = throw windup (5), 3 = smash windup (4) + impact (1)
void Golem::updateAnimation()
{
    int row = 0;
    int col = 0;

    switch (state)
    {
    case State::Idle:
        row = 0;
        col = static_cast<int>(anim_timer * 8.f) % 10;   // 8 pictures per second, looping
        break;
    case State::Walk:
        row = 1;
        col = static_cast<int>(anim_timer * 10.f) % 10;
        break;
    case State::ThrowWindup:
        row = 2;
        // Spread the 5 pictures evenly over the 0.6 second windup
        // State animation ke 0.6 second ko 5 frames mein baanta; elapsed / duration x frame count.
        col = int(state_timer / 0.6f * 5.f);
        if (col > 4) col = 4;
        break;
    case State::SmashWindup:
        row = 3;
        // Spread the first 4 pictures evenly over the 0.8 second windup
        // Smash animation = state time / 0.8 sec x 4 frames; if check last frame par limit lagata hai.
        col = int(state_timer / 0.8f * 4.f);
        if (col > 3) col = 3;
        break;
    case State::Recover:
        if (last_attack_was_smash && state_timer < 0.35f)
        {
            row = 3;
            col = 4;                 // the ground-smash impact with dust
        }
        else
        {
            row = 0;
            col = static_cast<int>(anim_timer * 8.f) % 10;
        }
        break;
    }

    // Sprite sheet crop X = column x frame width; Y = row x frame height.
    sprite.setTextureRect(sf::IntRect({ col * cell_width, row * cell_height }, { cell_width, cell_height }));
    sprite.setOrigin({ cell_width / 2.f, feet_y });                 // the golem's feet
    // Facing -1 ho to sprite horizontally reverse hota hai; +1 ho to normal.
    sprite.setScale({ sprite_scale * facing, sprite_scale });   // the sheet faces right
    // Put the feet at the bottom middle of the hitbox
    sprite.setPosition({ shape.getPosition().x + shape.getSize().x / 2.f,
                         shape.getPosition().y + shape.getSize().y });
    sprite.setColor(hurt_timer > 0.f ? sf::Color(255, 150, 150) : sf::Color::White);
}

void Golem::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;
    window.draw(sprite);
}

// Throws a rock in a curve (an arc) that lands where the player is standing now
void Golem::throwRock(sf::Vector2f target, EnemyWorld& world)
{
    // Golem rock start X = golem center + facing direction x 40 pixels.
    sf::Vector2f start(getCenter().x + facing * 40.f, shape.getPosition().y + 25.f);
    const float rock_gravity = 900.f;
    float x_gap = target.x - start.x;   // how far the rock must travel sideways
    float y_gap = target.y - start.y;   // how far up/down the target is
    // Far away = longer flight time = higher arc
    // Rock flight time ka base estimate horizontal distance / 450; if checks 0.7 se 1.5 seconds ke beech rakhte hain.
    float flight_time = std::abs(x_gap) / 450.f;
    if (flight_time < 0.7f) flight_time = 0.7f;
    if (flight_time > 1.5f) flight_time = 1.5f;

    // Sideways speed: simply distance / time
    // Rock ki horizontal speed = target tak X distance / allowed flight time.
    float speed_x = x_gap / flight_time;
    // Up/down speed: gravity pulls the rock down while it flies, so we throw it upward
    // just enough to still arrive at the target after flight_time seconds.
    // (From: y_gap = speed_y * time + 0.5 * gravity * time * time)
    // Projectile motion: gravity displacement = 1/2 x gravity x time squared; yGap se minus karke initial Y speed nikalte hain.
    float speed_y = (y_gap - 0.5f * rock_gravity * flight_time * flight_time) / flight_time;

    world.projectiles.push_back(
        Projectile(start, { 36.f, 36.f }, { speed_x, speed_y }, rock_gravity, 1,
            sf::Color(120, 110, 100), 4.f, true));   // true = draw the rock picture
}

// Stone spikes pop out on both sides, one after another, moving outward
void Golem::smashGround(EnemyWorld& world)
{
    float ground_y = shape.getPosition().y + shape.getSize().y;
    float center_x = getCenter().x;
    for (int i = 1; i <= 6; i++)
    {
        // Ground spikes ki distance = index x 75 + 20; har agla spike aage spawn hota hai.
        float offset = i * 75.f + 20.f;   // distance from the golem
        // Har spike ka wait = index x 0.09 sec, is se waves mein spikes nikalte hain.
        float spawn_delay = i * 0.09f;    // farther spikes pop out a little later
        world.spikes.push_back(Spike(center_x - offset, ground_y, spawn_delay));   // left side
        world.spikes.push_back(Spike(center_x + offset, ground_y, spawn_delay));   // right side
    }
}

// BurstSpirit ke functions yahan hain.
namespace
{
    const sf::Color spirit_color(170, 120, 255);

    // One shared copy of the picture for every spirit (so it is loaded only once)
    const sf::Texture& burstSpiritTexture()
    {
        static sf::Texture texture;
        static bool tried = false;
        if (!tried)
        {
            tried = true;
            if (!texture.loadFromFile("Assets/Sprites/Enemies/BurstSpirit/burst_spirit_sheet.png"))
                std::cout << "Could not load burst_spirit_sheet.png\n";
        }
        return texture;
    }
}

BurstSpirit::BurstSpirit(float x, float y)
    : Enemy({ x, y }, { 30.f, 30.f }, spirit_color, 2), sprite(burstSpiritTexture())
{
    updateAnimation();   // show the first frame at once (otherwise the whole sprite sheet is drawn before the first update)
}

void BurstSpirit::update(float delta_time, const std::vector<sf::RectangleShape>&,
    sf::Vector2f player_center, EnemyWorld&)
{
    if (!alive)
        return;

    updateHurtFlash(delta_time);
    bob_timer += delta_time;
    anim_timer += delta_time;
    state_timer += delta_time;
    shape.move({ consumeKnockback(delta_time), 0.f });

    // ---- Is the player standing still? ----
    // Speed = distance moved since last frame / time passed
    float player_speed = 1000.f; // pretend "moving" on the very first frame
    if (has_last_player_pos && delta_time > 0.f)
    {
        sf::Vector2f moved = player_center - last_player_center;
        // 2D distance formula sqrt(dx^2 + dy^2); movement ko seconds se divide karke player speed milti hai.
        player_speed = std::sqrt(moved.x * moved.x + moved.y * moved.y) / delta_time;
    }
    last_player_center = player_center;
    has_last_player_pos = true;

    if (player_speed < 20.f)
        still_timer += delta_time;
    else
        still_timer = 0.f; // moving resets the countdown

    // ---- Distance to the player ----
    sf::Vector2f to_player = player_center - getCenter();
    // Player tak seedhi distance = sqrt(horizontal gap squared + vertical gap squared).
    float distance_to_player = std::sqrt(to_player.x * to_player.x + to_player.y * to_player.y);

    if (distance_to_player <= sight_range)
        noticed = true;

    switch (state)
    {
    case State::Chase:
        base_color = spirit_color;
        if (noticed)
        {
            // Look at the player (the float pictures face right)
            facing = (to_player.x >= 0.f) ? 1.f : -1.f;
            // Fly toward the player (to_player / distance = direction with length 1)
            if (distance_to_player > 40.f)
                // Vector ko distance se divide = unit direction; phir speed x dt karne se Spirit player ki taraf move hota hai.
                shape.move(to_player / distance_to_player * chase_speed * delta_time * (1.f + 0.15f * (difficulty_level - 1)));

            // Gentle floating up and down
            // Sine wave -1 to +1 ke beech upar/neeche move karti hai; 6 frequency, 25 strength, dt frame time hai.
            shape.move({ 0.f, std::sin(bob_timer * 6.f) * 25.f * delta_time });

            // Close AND player stopped for a bit = start the countdown
            if (distance_to_player <= trigger_range && still_timer >= still_time_needed)
            {
                state = State::Fuse;
                state_timer = 0.f;
            }
        }
        break;

    case State::Fuse:
        // Flash red/white. The spirit stops and waits, so you can run away or kill it.
        // 12 changes/sec ki blinking pattern; %2 se alternate states milti hain.
        base_color = (static_cast<int>(state_timer * 12.f) % 2 == 0)
            ? sf::Color(255, 80, 80) : sf::Color::White;
        if (state_timer >= fuse_time)
        {
            state = State::Exploding;
            state_timer = 0.f;
        }
        break;

    case State::Exploding:
        base_color = sf::Color(255, 160, 50);
        if (state_timer >= explosion_time)
            alive = false;
        break;
    }

    updateAnimation();
}

// Pick which picture of burst_spirit_sheet.png to show
void BurstSpirit::updateAnimation()
{
    int row = 0;
    int col = 0;
    float scale = spirit_scale;

    switch (state)
    {
    case State::Chase:
        if (!noticed)
        {
            row = 0;
            col = static_cast<int>(anim_timer * 8.f) % 6;     // calm floating
        }
        else
        {
            row = 1;
            col = static_cast<int>(anim_timer * 10.f) % 6;    // flying at the player
        }
        break;
    case State::Fuse:
        row = 2;
        // The 6 charge pictures are spread over the fuse time; the last (glowing) one stays
        col = int(state_timer / fuse_time * 6.f);
        if (col > 5) col = 5;
        break;
    case State::Exploding:
        scale = boom_scale;
        if (state_timer < explosion_hurt_time)
        {
            row = 3;   // the bright burst (this part hurts)
            col = int(state_timer / explosion_hurt_time * 4.f);
        }
        else
        {
            row = 4;   // the explosion fading away
            col = int((state_timer - explosion_hurt_time) / (explosion_time - explosion_hurt_time) * 4.f);
        }
        if (col > 3) col = 3;
        break;
    }

    sprite.setTextureRect(sf::IntRect({ col * cell_size, row * cell_size }, { cell_size, cell_size }));
    sprite.setOrigin({ cell_size / 2.f, cell_size / 2.f });
    sprite.setScale({ scale * facing, scale });   // the sheet faces right
    sprite.setPosition(getCenter());
    sprite.setColor(hurt_timer > 0.f ? sf::Color(255, 140, 140) : sf::Color::White);
}

void BurstSpirit::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;
    window.draw(sprite);
    drawAttackBox(window);
}

// Can't be killed during the boom itself
void BurstSpirit::takeDamage(int amount, int strike_id, float from_direction)
{
    if (state == State::Exploding)
        return;
    Enemy::takeDamage(amount, strike_id, from_direction);
}

// The explosion is a square around the spirit
bool BurstSpirit::getAttackHitbox(sf::FloatRect& box) const
{
    // Only the bright burst hurts; the fading smoke after it is harmless
    if (state != State::Exploding || state_timer >= explosion_hurt_time)
        return false;

    sf::Vector2f center = getCenter();
    box = sf::FloatRect({ center.x - explosion_radius, center.y - explosion_radius },
        // Explosion ka full diameter = radius x 2; rectangle poore blast ko cover karta hai.
        { explosion_radius * 2.f, explosion_radius * 2.f });
    return true;
}

// Phoenix ke functions yahan hain.
#include <iostream>

namespace
{
    const sf::Color normal_color(255, 150, 40);
}

Phoenix::Phoenix(float x, float y)
    : Enemy({ x, y }, { 60.f, 44.f }, normal_color, 3), sprite(texture)
{
    if (!texture.loadFromFile("Assets/Sprites/Enemies/Phoenix/phoenix_sheet.png"))
        std::cout << "Could not load phoenix_sheet.png\n";
    sprite.setTexture(texture, true);
    updateAnimation();   // show the first frame at once (otherwise the whole sprite sheet is drawn before the first update)
}

// Fly to a spot in front of the player, a bit above them
void Phoenix::hoverNear(sf::Vector2f player_center, float delta_time)
{
    sf::Vector2f center = getCenter();

    // Stay on whichever side of the player we are already on
    float side = (center.x < player_center.x) ? -1.f : 1.f;
    // Phoenix hover X = player center +/- hover distance, side direction decide karta hai.
    float target_x = player_center.x + side * hover_distance;
    // Phoenix player se hover_height upar rehta hai; if check target Y ko 440 se neeche nahi jaane deta.
    float target_y = player_center.y - hover_height;
    if (target_y > 440.f) target_y = 440.f; // stay above the floor

    sf::Vector2f to_target(target_x - center.x, target_y - center.y);
    // Hover target tak Pythagoras distance = sqrt(dx^2 + dy^2).
    float distance_to_target = std::sqrt(to_target.x * to_target.x + to_target.y * to_target.y);
    if (distance_to_target > 4.f)
    {
        // Move toward the target, but never past it
        // Is frame step = speed x dt, magar remaining distance se zyada nahi, warna overshoot hoga.
        float step = hover_speed * delta_time * (1.f + 0.15f * (difficulty_level - 1));
        if (step > distance_to_target) step = distance_to_target;
        // Direction vector normalize karke step se multiply, Phoenix target tak smoothly jata hai.
        shape.move(to_target / distance_to_target * step);
    }
}

void Phoenix::update(float delta_time, const std::vector<sf::RectangleShape>&,
    sf::Vector2f player_center, EnemyWorld& world)
{
    if (!alive)
        return;

    updateHurtFlash(delta_time);
    anim_timer += delta_time;

    // Death animation: just play it, then really disappear
    if (state == State::Dying)
    {
        state_timer += delta_time;
        if (state_timer >= dying_time)
            alive = false;
        updateAnimation();
        return;
    }

    shape.move({ consumeKnockback(delta_time), 0.f });
    state_timer += delta_time;

    sf::Vector2f center = getCenter();
    sf::Vector2f to_player = player_center - center;

    // Flow: Hover -> (BreathWindup -> Breathing) or (ClawWindup -> ClawStrike) -> Recover -> Hover
    switch (state)
    {
    case State::Hover:
        facing = (to_player.x >= 0.f) ? 1.f : -1.f;
        hoverNear(player_center, delta_time);
        attack_cooldown -= delta_time;
        if (attack_cooldown <= 0.f)
        {
            // Close = claws, far = fire breath
            bool player_is_close = std::abs(to_player.x) < claw_range && std::abs(to_player.y) < 160.f;
            state = player_is_close ? State::ClawWindup : State::BreathWindup;
            state_timer = 0.f;
        }
        break;

    case State::BreathWindup:
        facing = (to_player.x >= 0.f) ? 1.f : -1.f;
        if (state_timer >= 0.55f)
        {
            // Aim once, now. If you move, the fire misses!
            breath_angle = std::atan2(to_player.y, to_player.x);
            breath_shot_timer = 0.f;
            state = State::Breathing;
            state_timer = 0.f;
        }
        break;

    case State::Breathing:
        breath_shot_timer -= delta_time;
        if (breath_shot_timer <= 0.f)
        {
            breath_shot_timer = 0.07f;   // one fireball every 0.07 seconds
            // The fire wiggles a little up and down
            // Fire ka angle sine se halka left/right hilta hai: frequency 14 aur deviation 0.15 radian.
            float angle = breath_angle + std::sin(state_timer * 14.f) * 0.15f;
            sf::Vector2f direction(std::cos(angle), std::sin(angle));
            world.projectiles.push_back(
                // Fire projectile ki velocity = normalized direction x 430 units/second.
                Projectile(center + direction * 26.f, { 16.f, 16.f }, direction * 430.f, 0.f, 1,
                    sf::Color(255, 140, 30), 0.7f));
        }
        if (state_timer >= 1.0f)
        {
            attack_cooldown = 2.f;
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    case State::ClawWindup:
        facing = (to_player.x >= 0.f) ? 1.f : -1.f;
        if (state_timer >= 0.4f)
        {
            // Remember the direction to the player, so the swoop is a straight line
            float length = std::sqrt(to_player.x * to_player.x + to_player.y * to_player.y);
            if (length > 0.f)
                // Claw lunge direction = player tak vector / uski length; is ka magnitude 1 hota hai.
                lunge_dir = to_player / length;
            else
                lunge_dir = { facing, 0.f };
            facing = (lunge_dir.x >= 0.f) ? 1.f : -1.f;
            state = State::ClawStrike;
            state_timer = 0.f;
        }
        break;

    case State::ClawStrike:
        // Phoenix ka lunge distance = normalized direction x speed x frame seconds.
        shape.move(lunge_dir * claw_lunge_speed * delta_time); // swoops at the player
        if (state_timer >= 0.25f)
        {
            attack_cooldown = 1.2f;
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    case State::Recover:
        hoverNear(player_center, delta_time);
        if (state_timer >= 0.7f)
        {
            state = State::Hover;
            state_timer = 0.f;
        }
        break;

    case State::Dying:
        break;
    }

    updateAnimation();
}

// Pick which picture of the sheet to show.
// Sheet rows: 0 = front idle, 1 = flying, 2 = fire breath, 3 = claw swoop, 4 = hurt / death
void Phoenix::updateAnimation()
{
    int row = 1;
    int col = 0;

    switch (state)
    {
    case State::Hover:
    case State::Recover:
        row = 1;
        col = static_cast<int>(anim_timer * 12.f) % 6;       // flapping loop
        break;
    case State::BreathWindup:
        row = 2;
        col = int(state_timer / 0.55f * 3.f);
        if (col > 2) col = 2;
        break;
    case State::Breathing:
        row = 2;
        col = 3 + static_cast<int>(state_timer * 12.f) % 2;  // the two "spitting fire" pictures
        break;
    case State::ClawWindup:
        row = 3;
        col = int(state_timer / 0.4f * 3.f);
        if (col > 2) col = 2;
        break;
    case State::ClawStrike:
        row = 3;
        col = int(state_timer / 0.25f * 3.f);
        if (col > 2) col = 2;
        col = 3 + col;
        break;
    case State::Dying:
        row = 4;
        // Death animation progress = elapsed / dying duration; x5 se frame sequence select hoti hai.
        col = int(state_timer / dying_time * 5.f);
        if (col > 4) col = 4;
        col = 5 + col;
        break;
    }

    // Short "ouch" picture when hit
    if (hurt_timer > 0.f && state != State::Dying)
    {
        row = 4;
        col = 1;
    }

    sprite.setTextureRect(sf::IntRect({ col * frame_width, row * frame_height }, { frame_width, frame_height }));
    sprite.setOrigin({ frame_width / 2.f, frame_height / 2.f });
    sprite.setScale({ sprite_scale * facing, sprite_scale });   // the sheet faces right
    sprite.setPosition(getCenter());
}

void Phoenix::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;

    window.draw(sprite);

    // Red see-through box = "this area can hurt you right now"
    sf::FloatRect box;
    if (getAttackHitbox(box))
    {
        sf::RectangleShape attack_box(box.size);
        attack_box.setPosition(box.position);
        attack_box.setFillColor(sf::Color(255, 0, 0, 100));
        window.draw(attack_box);
    }
}

// When health runs out, play the death animation first instead of vanishing
void Phoenix::takeDamage(int amount, int strike_id, float from_direction)
{
    if (state == State::Dying)
        return;

    Enemy::takeDamage(amount, strike_id, from_direction);

    if (!alive)
    {
        alive = true;   // stay in the game while the death animation plays
        state = State::Dying;
        state_timer = 0.f;
    }
}

// The claws only hurt during the swoop
bool Phoenix::getAttackHitbox(sf::FloatRect& box) const
{
    if (state != State::ClawStrike)
        return false;
    sf::Vector2f center = getCenter();
    const float box_width = 90.f;
    const float box_height = 80.f;
    // The claw box sits in front of the phoenix, on the side it is facing
    float box_x = (facing > 0.f) ? center.x : center.x - box_width;
    // Attack box ka top = enemy center Y - half attack height.
    box = sf::FloatRect({ box_x, center.y - box_height / 2.f }, { box_width, box_height });
    return true;
}


// ===========================================================================
// Skeleton
// ===========================================================================
namespace
{
    const sf::Color skeleton_bone(225, 220, 195);

    const sf::Texture& skeletonTexture()
    {
        static sf::Texture texture;
        static bool tried = false;
        if (!tried)
        {
            tried = true;
            if (!texture.loadFromFile("Assets/Sprites/Enemies/Skeleton/skeleton_sheet.png"))
                std::cout << "Could not load skeleton_sheet.png\n";
        }
        return texture;
    }
}

Skeleton::Skeleton(float x, float y)
    : Enemy({ x, y }, { 44.f, 84.f }, skeleton_bone, 3), sprite(skeletonTexture())
{
    knockback_speed = 150.f;
    updateAnimation();   // show the first frame at once (otherwise the whole sprite sheet is drawn before the first update)
}

void Skeleton::update(float delta_time, const std::vector<sf::RectangleShape>& solids,
    sf::Vector2f player_center, EnemyWorld&)
{
    if (!alive)
        return;

    updateHurtFlash(delta_time);
    anim_timer += delta_time;
    state_timer += delta_time;
    if (slash_cooldown > 0.f) slash_cooldown -= delta_time;
    if (leap_cooldown > 0.f) leap_cooldown -= delta_time;

    moveHorizontal(consumeKnockback(delta_time), solids);

    sf::Vector2f center = getCenter();
    float dx = player_center.x - center.x;
    float dy = player_center.y - center.y;
    float dist = std::abs(dx);
    bool on_floor = (velocity_y == 0.f);

    switch (state)
    {
    case State::Idle:
    case State::Walk:
        base_color = skeleton_bone;
        if (dist > sight_range)
        {
            state = State::Idle;
            break;
        }
        facing = (dx >= 0.f) ? 1.f : -1.f;

        if (dist <= slash_range && std::abs(dy) < 100.f && slash_cooldown <= 0.f)
        {
            // NEAR: slash
            state = State::SlashWindup;
            last_attack_was_leap = false;
            state_timer = 0.f;
        }
        else if (dist >= leap_min_range && dist <= leap_max_range
            && leap_cooldown <= 0.f && on_floor)
        {
            // FAR: jump attack
            state = State::LeapWindup;
            last_attack_was_leap = true;
            state_timer = 0.f;
        }
        // Skeleton desired approach distance slash range ka 80% hai; is se attack reach se thora paas aata hai.
        else if (dist > slash_range * 0.8f)
        {
            state = State::Walk;
            moveHorizontal(facing * walk_speed * delta_time * (1.f + 0.15f * (difficulty_level - 1)), solids);
        }
        break;

    case State::SlashWindup:
        base_color = sf::Color(255, 230, 120);   // yellow = about to slash
        if (state_timer >= slash_windup_time)
        {
            state = State::SlashStrike;
            state_timer = 0.f;
        }
        break;

    case State::SlashStrike:
        base_color = sf::Color(255, 150, 80);
        if (state_timer >= slash_active_time)
        {
            slash_cooldown = 1.2f;
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    case State::LeapWindup:
        base_color = sf::Color(140, 200, 255);   // blue = about to jump
        facing = (dx >= 0.f) ? 1.f : -1.f;
        if (state_timer >= leap_windup_time)
        {
            // Aim at where the player is right now. If the player moves, the jump misses.
            // Jump physics: total air time approx 2 x upward jump speed / gravity (981).
            float flight_time = 2.f * leap_jump_speed / 981.f;
            // Horizontal jump speed = remaining X gap / flight time; if checks maximum left/right speed limit lagate hain.
            leap_speed_x = dx / flight_time;
            if (leap_speed_x < -leap_max_speed_x) leap_speed_x = -leap_max_speed_x;
            if (leap_speed_x > leap_max_speed_x) leap_speed_x = leap_max_speed_x;
            facing = (leap_speed_x >= 0.f) ? 1.f : -1.f;
            velocity_y = -leap_jump_speed;
            state = State::Leap;
            state_timer = 0.f;
        }
        break;

    case State::Leap:
        base_color = sf::Color(255, 150, 80);
        if (moveHorizontal(leap_speed_x * delta_time, solids))
            leap_speed_x = 0.f;   // hit a wall: drop straight down
        break;

    case State::Recover:
        base_color = skeleton_bone;
        if (state_timer >= recover_time)
        {
            state = State::Idle;
            state_timer = 0.f;
        }
        break;
    }

    applyGravity(delta_time, solids);

    // Landed after the jump
    if (state == State::Leap && state_timer > 0.15f && velocity_y == 0.f)
    {
        leap_cooldown = 2.f;
        state = State::Recover;
        state_timer = 0.f;
    }

    updateAnimation();
}

// Pick which picture of skeleton_sheet.png to show.
// Row 0 = bone swing (7 pictures), row 1 = dash jump (9 pictures)
void Skeleton::updateAnimation()
{
    int row = 0;
    int col = 0;
    float bob = 0.f;   // little up/down while walking

    switch (state)
    {
    case State::Idle:
        // Breathing: switch slowly between the idle picture and the "recovery 2" picture (same pose)
        col = (static_cast<int>(anim_timer * 3.f) % 2 == 0) ? 0 : 6;
        break;
    case State::Walk:
        col = (static_cast<int>(anim_timer * 6.f) % 2 == 0) ? 0 : 6;
        bob = std::abs(std::sin(anim_timer * 9.f)) * 3.f;
        break;
    case State::SlashWindup:
        // wind-up 1 (arm goes up), then wind-up 2 (pulls back)
        col = (state_timer < slash_windup_time * 0.55f) ? 1 : 2;
        break;
    case State::SlashStrike:
        // swing 1, then swing 2 with the motion blur
        col = (state_timer < slash_active_time * 0.5f) ? 3 : 4;
        break;
    case State::LeapWindup:
        row = 1;
        col = (state_timer < leap_windup_time * 0.5f) ? 0 : 1;   // crouch 1, crouch 2 (glowing)
        break;
    case State::Leap:
    {
        row = 1;
        // The jump lasts about 2 x jump speed / gravity seconds; p = how far through the jump we are
        float p = state_timer / (2.f * leap_jump_speed / 981.f);
        if (p < 0.2f)       col = 2;   // leap
        else if (p < 0.45f) col = 3;   // mid-air
        else if (p < 0.7f)  col = 4;   // dash arc
        else                col = 5;   // impact slash (red glow)
        break;
    }
    case State::Recover:
        if (last_attack_was_leap)
        {
            row = 1;
            if (state_timer < 0.2f)       col = 6;   // landing
            else if (state_timer < 0.4f)  col = 7;   // stumbles
            else                          col = 8;   // back to idle
        }
        else
        {
            col = (state_timer < 0.3f) ? 5 : 6;      // recovery 1, recovery 2
        }
        break;
    }

    sprite.setTextureRect(sf::IntRect({ col * cell_width, row * cell_height }, { cell_width, cell_height }));
    sprite.setOrigin({ cell_width / 2.f, feet_y });
    float flip = sheet_faces_right ? facing : -facing;
    sprite.setScale({ sprite_scale * flip, sprite_scale });
    // Feet at the bottom middle of the hitbox
    sprite.setPosition({ shape.getPosition().x + shape.getSize().x / 2.f,
                         shape.getPosition().y + shape.getSize().y - bob });
    sprite.setColor(hurt_timer > 0.f ? sf::Color(255, 150, 150) : sf::Color::White);
}

void Skeleton::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;

    window.draw(sprite);
    drawAttackBox(window);
}

// Slash: a box in front of the skeleton. Jump: the whole body (a bit bigger) hurts while flying.
bool Skeleton::getAttackHitbox(sf::FloatRect& box) const
{
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f size = shape.getSize();

    if (state == State::SlashStrike)
    {
        const float w = 80.f;
        const float h = 60.f;
        float x = (facing > 0.f) ? pos.x + size.x : pos.x - w;
        box = sf::FloatRect({ x, pos.y + 10.f }, { w, h });
        return true;
    }
    if (state == State::Leap)
    {
        box = sf::FloatRect({ pos.x - 10.f, pos.y - 5.f }, { size.x + 20.f, size.y + 10.f });
        return true;
    }
    return false;
}



// ===========================================================================
// DraconicLord (final boss)
// ===========================================================================
namespace
{
    const sf::Color lord_color(140, 35, 45);          // dark dragon red
    const sf::Color lord_scale_color(90, 200, 140);   // the flying dragon scales
    const sf::Color lord_fire_color(255, 140, 30);
    const float lord_pi = 3.14159265f;

    const sf::Texture& lordTexture()
    {
        static sf::Texture texture;
        static bool tried = false;
        if (!tried)
        {
            tried = true;
            if (!texture.loadFromFile("Assets/Sprites/Enemies/DraconicLord/draconic_lord_sheet.png"))
                std::cout << "Could not load draconic_lord_sheet.png\n";
        }
        return texture;
    }

    const sf::Texture& lordFireTexture()
    {
        static sf::Texture texture;
        static bool tried = false;
        if (!tried)
        {
            tried = true;
            if (!texture.loadFromFile("Assets/Sprites/Enemies/DraconicLord/lord_fire_strips.png"))
                std::cout << "Could not load lord_fire_strips.png\n";
        }
        return texture;
    }
}

DraconicLord::DraconicLord(float x, float y)
    : Enemy({ x, y }, { 56.f, 104.f }, lord_color, 30), sprite(lordTexture())
{
    knockback_speed = 40.f;   // a boss barely gets pushed
}

// Pick one of the 5 attacks (not the one used last time) and start its warning
void DraconicLord::chooseAttack()
{
    int attack = (last_attack + 1 + std::rand() % 4) % 5;
    last_attack = attack;
    state_timer = 0.f;

    switch (attack)
    {
    case 0: state = State::ScalesWindup; break;
    case 1: state = State::StrikeWindup; break;
    case 2: state = State::FireballWindup; break;
    case 3: state = State::BladeWindup; break;
    default: state = State::FireWindup; break;
    }
}

// Never walk or dash out of the room
void DraconicLord::clampToArena()
{
    sf::Vector2f pos = shape.getPosition();
    float width = shape.getSize().x;
    if (pos.x < arena_left)
        shape.setPosition({ arena_left, pos.y });
    if (pos.x + width > arena_right)
        shape.setPosition({ arena_right - width, pos.y });
}

void DraconicLord::update(float delta_time, const std::vector<sf::RectangleShape>& solids,
    sf::Vector2f player_center, EnemyWorld& world)
{
    if (!alive)
        return;

    updateHurtFlash(delta_time);
    anim_timer += delta_time;
    state_timer += delta_time;
    if (attack_cooldown > 0.f) attack_cooldown -= delta_time;

    moveHorizontal(consumeKnockback(delta_time), solids);

    sf::Vector2f center = getCenter();
    float dx = player_center.x - center.x;

    switch (state)
    {
    case State::Walk:
        base_color = lord_color;
        facing = (dx >= 0.f) ? 1.f : -1.f;
        moving = std::abs(dx) > stop_range;
        if (moving)
            moveHorizontal(facing * walk_speed * delta_time, solids);
        if (attack_cooldown <= 0.f)
            chooseAttack();
        break;

    // ---- 0: Dragon scales ----
    case State::ScalesWindup:
        base_color = sf::Color(120, 230, 170);   // green = scales are coming
        facing = (dx >= 0.f) ? 1.f : -1.f;
        if (state_timer >= scales_windup_time)
        {
            throwScales(world);
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    // ---- 1: Draconic strike (dash slash) ----
    case State::StrikeWindup:
        base_color = sf::Color(255, 230, 120);   // yellow = about to dash (dodge through it!)
        facing = (dx >= 0.f) ? 1.f : -1.f;       // aims at the player, then the direction is locked
        if (state_timer >= strike_windup_time)
        {
            state = State::StrikeDash;
            state_timer = 0.f;
        }
        break;

    case State::StrikeDash:
        base_color = sf::Color(255, 150, 80);
        moveHorizontal(facing * strike_speed * delta_time, solids);
        if (state_timer >= strike_dash_time)
        {
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    // ---- 2: Fireball ----
    case State::FireballWindup:
        base_color = sf::Color(255, 170, 60);    // orange = about to throw fire
        facing = (dx >= 0.f) ? 1.f : -1.f;
        if (state_timer >= fireball_windup_time)
        {
            throwFireball(player_center, world);
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    // ---- 3: Planted blade ----
    case State::BladeWindup:
        base_color = sf::Color(190, 200, 230);   // steel = about to throw the blade
        facing = (dx >= 0.f) ? 1.f : -1.f;
        if (state_timer >= blade_windup_time)
        {
            // The blade lands where the player stands right now. Move away before it explodes!
            blade_x = player_center.x;
            ground_y = shape.getPosition().y + shape.getSize().y;
            state = State::BladePlanted;
            state_timer = 0.f;
        }
        break;

    case State::BladePlanted:
        base_color = lord_color;
        if (state_timer >= blade_planted_time)
        {
            state = State::BladeExplode;
            state_timer = 0.f;
        }
        break;

    case State::BladeExplode:
        base_color = lord_color;
        if (state_timer >= blade_explode_time)
        {
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    // ---- 4: Burning ground ----
    case State::FireWindup:
        base_color = sf::Color(255, 70, 40);     // red = the floor is about to burn
        ground_y = shape.getPosition().y + shape.getSize().y;
        if (state_timer >= fire_windup_time)
        {
            state = State::FireBurning;
            state_timer = 0.f;
        }
        break;

    case State::FireBurning:
        base_color = lord_color;
        if (state_timer >= fire_burn_time)
        {
            state = State::Recover;
            state_timer = 0.f;
        }
        break;

    case State::Recover:
        base_color = lord_color;
        if (state_timer >= recover_time)
        {
            attack_cooldown = attack_pause;
            state = State::Walk;
            state_timer = 0.f;
        }
        break;
    }

    clampToArena();
    applyGravity(delta_time, solids);
    updateAnimation();
}

// Pick which picture of draconic_lord_sheet.png the body shows.
// Rows: 0 = idle + walk, 1 = dash slash, 2 = dragon scales, 3 = fireball, 4 = mine, 5 = burning ground
void DraconicLord::updateAnimation()
{
    int row = 0;
    int col = 0;
    const int side_stand_col = 2;   // row 0, first walking picture = a calm side-on stance

    switch (state)
    {
    case State::Walk:
        row = 0;
        if (moving)
            col = 2 + static_cast<int>(anim_timer * 8.f) % 5;   // the 5 walking pictures
        else
            col = static_cast<int>(anim_timer * 2.5f) % 2;      // standing, facing us
        break;

    // 0: dragon scales (row 2)
    case State::ScalesWindup:
        row = 2;
        col = (state_timer < 0.25f) ? 0 : (state_timer < 0.5f ? 1 : 2);
        break;

    // 1: dash slash (row 1)
    case State::StrikeWindup:
        row = 1;
        col = (state_timer < 0.2f) ? 0 : (state_timer < 0.38f ? 1 : 2);
        break;
    case State::StrikeDash:
        row = 1;
        col = (state_timer < strike_dash_time * 0.5f) ? 3 : 4;
        break;

    // 2: fireball (row 3)
    case State::FireballWindup:
        row = 3;
        col = (state_timer < 0.2f) ? 0 : (state_timer < 0.4f ? 1 : 2);
        break;

    // 3: mine (row 4). The mine itself is drawn in draw(); the boss crouches while he throws it.
    case State::BladeWindup:
        row = 4;
        col = 2;
        break;
    case State::BladePlanted:
    case State::BladeExplode:
        row = 0;
        col = side_stand_col;
        break;

    // 4: burning ground (row 5)
    case State::FireWindup:
        row = 5;
        col = (state_timer < 0.3f) ? 0 : (state_timer < 0.6f ? 1 : 2);   // arms up, roaring
        break;
    case State::FireBurning:
        row = 5;
        col = 3;   // hand on the floor
        break;

    case State::Recover:
        switch (last_attack)
        {
        case 0:
            row = 2;
            if (state_timer < 0.12f)      col = 3;   // the claws fly out
            else if (state_timer < 0.24f) col = 6;   // burst
            else if (state_timer < 0.45f) col = 7;
            else                          col = 8;
            break;
        case 1:
            if (state_timer < 0.35f) { row = 1; col = 5; }
            else { row = 0; col = side_stand_col; }
            break;
        case 2:
            row = 3;
            if (state_timer < 0.15f)      col = 3;
            else if (state_timer < 0.3f)  col = 5;
            else if (state_timer < 0.5f)  col = 6;
            else                          col = 7;
            break;
        case 4:
            if (state_timer < 0.3f) { row = 5; col = 3; }
            else { row = 0; col = side_stand_col; }
            break;
        default:
            row = 0;
            col = side_stand_col;
            break;
        }
        break;
    }

    body_row = row;
    body_col = col;
}

// A ring of scales flying out in every direction
void DraconicLord::throwScales(EnemyWorld& world)
{
    sf::Vector2f center = getCenter();
    for (int i = 0; i < scale_count; i++)
    {
        // Angles are spread evenly around the full circle (2 x pi divided by the number of scales)
        float angle = i * 2.f * lord_pi / scale_count;
        sf::Vector2f direction(std::cos(angle), std::sin(angle));
        world.projectiles.push_back(
            Projectile(center + direction * 40.f, { scale_size, scale_size }, direction * scale_speed,
                0.f, 1, lord_scale_color, scale_life, false, 3));   // 3 = claw shard picture
    }
}

// One big ball of fire, aimed at the player
void DraconicLord::throwFireball(sf::Vector2f player_center, EnemyWorld& world)
{
    sf::Vector2f start(getCenter().x + facing * 40.f, getCenter().y - 20.f);
    sf::Vector2f to_player = player_center - start;
    float length = std::sqrt(to_player.x * to_player.x + to_player.y * to_player.y);
    sf::Vector2f direction(facing, 0.f);
    if (length > 0.f)
        direction = to_player / length;
    world.projectiles.push_back(
        Projectile(start, { fireball_size, fireball_size }, direction * fireball_speed,
            0.f, 1, lord_fire_color, 3.f, false, 2));   // 2 = boss fireball picture
}

void DraconicLord::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;

    // ---- Warnings and effects that lie on the floor (drawn first, so the boss stands in front) ----
    if (state == State::BladePlanted || state == State::BladeExplode)
    {
        // The danger area slowly gets redder while the mine waits
        float fuse = state_timer / blade_planted_time;
        if (state == State::BladeExplode || fuse > 1.f)
            fuse = 1.f;
        sf::RectangleShape area({ explosion_width, explosion_height });
        area.setPosition({ blade_x - explosion_width / 2.f, ground_y - explosion_height });
        area.setFillColor(sf::Color(255, 60, 30, static_cast<unsigned char>(30 + 80 * fuse)));
        area.setOutlineColor(sf::Color(255, 90, 40, 160));
        area.setOutlineThickness(2.f);
        window.draw(area);

        // The mine (row 4 of the sheet). It pulses faster and glows redder before it goes off.
        sprite.setOrigin({ 123.f, 171.f });   // bottom middle of the mine picture
        sprite.setPosition({ blade_x, ground_y });
        if (state == State::BladePlanted)
        {
            float pulse = 1.f + 0.05f * std::sin(state_timer * (6.f + 14.f * fuse));
            sprite.setTextureRect(sf::IntRect({ 3 * cell_width, 4 * cell_height }, { cell_width, cell_height }));
            sprite.setScale({ 0.9f * pulse, 0.9f * pulse });
            sprite.setColor(sf::Color(255, static_cast<unsigned char>(255 - 110 * fuse),
                static_cast<unsigned char>(255 - 110 * fuse)));
        }
        else
        {
            // Smoke + flash first, then flying sparks
            int col = (state_timer < blade_explode_time * 0.5f) ? 6 : 7;
            sprite.setTextureRect(sf::IntRect({ col * cell_width, 4 * cell_height }, { cell_width, cell_height }));
            sprite.setScale({ 1.8f, 1.8f });
            sprite.setColor(sf::Color::White);
        }
        window.draw(sprite);
    }
    if (state == State::FireWindup)
    {
        // The whole floor glows, it flickers faster and faster
        int glow = 60 + static_cast<int>(60 * std::abs(std::sin(state_timer * 10.f)));
        sf::RectangleShape floor_glow({ arena_right - arena_left, fire_height });
        floor_glow.setPosition({ arena_left, ground_y - fire_height });
        floor_glow.setFillColor(sf::Color(255, 90, 30, static_cast<unsigned char>(glow)));
        window.draw(floor_glow);
    }
    if (state == State::FireBurning)
    {
        // Pictures of flames tiled along the whole floor. They overlap and are added on top of
        // the background, so the joins are not visible. Every tile runs its own frame.
        sf::Sprite flame(lordFireTexture());
        flame.setOrigin({ strip_width / 2.f, static_cast<float>(strip_height) });
        flame.setScale({ strip_scale, strip_scale });
        const float step = strip_width * strip_scale * 0.5f;
        int tile = 0;
        for (float x = arena_left; x < arena_right + step; x += step, tile++)
        {
            int frame = (static_cast<int>(state_timer * 10.f) + tile) % 4;
            flame.setTextureRect(sf::IntRect({ frame * strip_width, 0 }, { strip_width, strip_height }));
            flame.setPosition({ x, ground_y + 2.f });
            window.draw(flame, sf::BlendAdd);
        }
    }

    // ---- The boss himself ----
    sprite.setTextureRect(sf::IntRect({ body_col * cell_width, body_row * cell_height }, { cell_width, cell_height }));
    sprite.setOrigin({ cell_width / 2.f, feet_y });
    sprite.setScale({ sprite_scale * facing, sprite_scale });   // the sheet faces right
    // Feet at the bottom middle of the hitbox
    sprite.setPosition({ shape.getPosition().x + shape.getSize().x / 2.f,
                         shape.getPosition().y + shape.getSize().y });
    sprite.setColor(hurt_timer > 0.f ? sf::Color(255, 150, 150) : sf::Color::White);
    window.draw(sprite);

    drawAttackBox(window);
}

// Hurts only during: the dash slash, the blade explosion, and while the floor burns
bool DraconicLord::getAttackHitbox(sf::FloatRect& box) const
{
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f size = shape.getSize();

    if (state == State::StrikeDash)
    {
        // The body plus the slash reaching out in front of it
        float x = (facing > 0.f) ? pos.x : pos.x - strike_reach;
        box = sf::FloatRect({ x, pos.y - 5.f }, { size.x + strike_reach, size.y + 10.f });
        return true;
    }
    if (state == State::BladeExplode)
    {
        box = sf::FloatRect({ blade_x - explosion_width / 2.f, ground_y - explosion_height },
            { explosion_width, explosion_height });
        return true;
    }
    if (state == State::FireBurning)
    {
        box = sf::FloatRect({ arena_left, ground_y - fire_height },
            { arena_right - arena_left, fire_height });
        return true;
    }
    return false;
}
