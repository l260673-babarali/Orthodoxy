#include "../Classes/Enemy.h"

Enemy::Enemy(sf::Vector2f position, sf::Vector2f size, sf::Color color, int start_health)
    : base_color(color), health(start_health)
{
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
            if (move_x > 0.f)   // moving right: stop at the solid's left side
                shape.setPosition({ solid_box.position.x - enemy_box.size.x, enemy_box.position.y });
            else               // moving left: stop at the solid's right side
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

    velocity_y += gravity * delta_time;
    shape.move({ 0.f, velocity_y * delta_time });

    for (const auto& solid : solids)
    {
        sf::FloatRect enemy_box = shape.getGlobalBounds();
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (enemy_box.findIntersection(solid_box))
        {
            if (velocity_y > 0.f)   // falling: land on top of the solid
            {
                shape.setPosition({ enemy_box.position.x, solid_box.position.y - enemy_box.size.y });
                velocity_y = 0.f;
            }
            else if (velocity_y < 0.f)   // going up: bump the head on the underside
            {
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

void Enemy::takeDamage(int amount, int strike_id, float from_direction)
{
    health -= amount;
    last_hit_strike_id = strike_id;
    hurt_timer = 0.15f;
    knockback_timer = 0.15f;
    knockback_dir = from_direction;

    if (health <= 0)
        alive = false;
}

bool Enemy::isAlive() const { return alive; }
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
            moveHorizontal(facing * walk_speed * delta_time, solids);
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
        col = std::min(4, static_cast<int>(state_timer / 0.6f * 5.f));
        break;
    case State::SmashWindup:
        row = 3;
        // Spread the first 4 pictures evenly over the 0.8 second windup
        col = std::min(3, static_cast<int>(state_timer / 0.8f * 4.f));
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

    sprite.setTextureRect(sf::IntRect({ col * cell_width, row * cell_height }, { cell_width, cell_height }));
    sprite.setOrigin({ cell_width / 2.f, feet_y });                 // the golem's feet
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
    sf::Vector2f start(getCenter().x + facing * 40.f, shape.getPosition().y + 25.f);
    const float rock_gravity = 900.f;
    float x_gap = target.x - start.x;   // how far the rock must travel sideways
    float y_gap = target.y - start.y;   // how far up/down the target is
    // Far away = longer flight time = higher arc
    float flight_time = std::clamp(std::abs(x_gap) / 450.f, 0.7f, 1.5f);

    // Sideways speed: simply distance / time
    float speed_x = x_gap / flight_time;
    // Up/down speed: gravity pulls the rock down while it flies, so we throw it upward
    // just enough to still arrive at the target after flight_time seconds.
    // (From: y_gap = speed_y * time + 0.5 * gravity * time * time)
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
        float offset = i * 75.f + 20.f;   // distance from the golem
        float spawn_delay = i * 0.09f;    // farther spikes pop out a little later
        world.spikes.push_back(Spike(center_x - offset, ground_y, spawn_delay));   // left side
        world.spikes.push_back(Spike(center_x + offset, ground_y, spawn_delay));   // right side
    }
}

// BurstSpirit ke functions yahan hain.
namespace
{
    const sf::Color spirit_color(170, 120, 255);
}

BurstSpirit::BurstSpirit(float x, float y)
    : Enemy({ x, y }, { 30.f, 30.f }, spirit_color, 2)
{
}

void BurstSpirit::update(float delta_time, const std::vector<sf::RectangleShape>&,
    sf::Vector2f player_center, EnemyWorld&)
{
    if (!alive)
        return;

    updateHurtFlash(delta_time);
    bob_timer += delta_time;
    state_timer += delta_time;
    shape.move({ consumeKnockback(delta_time), 0.f });

    // ---- Is the player standing still? ----
    // Speed = distance moved since last frame / time passed
    float player_speed = 1000.f; // pretend "moving" on the very first frame
    if (has_last_player_pos && delta_time > 0.f)
    {
        sf::Vector2f moved = player_center - last_player_center;
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
    float distance_to_player = std::sqrt(to_player.x * to_player.x + to_player.y * to_player.y);

    if (distance_to_player <= sight_range)
        noticed = true;

    switch (state)
    {
    case State::Chase:
        base_color = spirit_color;
        if (noticed)
        {
            // Fly toward the player (to_player / distance = direction with length 1)
            if (distance_to_player > 40.f)
                shape.move(to_player / distance_to_player * chase_speed * delta_time);

            // Gentle floating up and down
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
    if (state != State::Exploding)
        return false;

    sf::Vector2f center = getCenter();
    box = sf::FloatRect({ center.x - explosion_radius, center.y - explosion_radius },
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
}

// Fly to a spot in front of the player, a bit above them
void Phoenix::hoverNear(sf::Vector2f player_center, float delta_time)
{
    sf::Vector2f center = getCenter();

    // Stay on whichever side of the player we are already on
    float side = (center.x < player_center.x) ? -1.f : 1.f;
    float target_x = player_center.x + side * hover_distance;
    float target_y = std::min(player_center.y - hover_height, 440.f); // stay above the floor

    sf::Vector2f to_target(target_x - center.x, target_y - center.y);
    float distance_to_target = std::sqrt(to_target.x * to_target.x + to_target.y * to_target.y);
    if (distance_to_target > 4.f)
    {
        // Move toward the target, but never past it
        float step = std::min(hover_speed * delta_time, distance_to_target);
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
            float angle = breath_angle + std::sin(state_timer * 14.f) * 0.15f;
            sf::Vector2f direction(std::cos(angle), std::sin(angle));
            world.projectiles.push_back(
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
                lunge_dir = to_player / length;
            else
                lunge_dir = { facing, 0.f };
            facing = (lunge_dir.x >= 0.f) ? 1.f : -1.f;
            state = State::ClawStrike;
            state_timer = 0.f;
        }
        break;

    case State::ClawStrike:
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
        col = std::min(2, static_cast<int>(state_timer / 0.55f * 3.f));
        break;
    case State::Breathing:
        row = 2;
        col = 3 + static_cast<int>(state_timer * 12.f) % 2;  // the two "spitting fire" pictures
        break;
    case State::ClawWindup:
        row = 3;
        col = std::min(2, static_cast<int>(state_timer / 0.4f * 3.f));
        break;
    case State::ClawStrike:
        row = 3;
        col = 3 + std::min(2, static_cast<int>(state_timer / 0.25f * 3.f));
        break;
    case State::Dying:
        row = 4;
        col = 5 + std::min(4, static_cast<int>(state_timer / dying_time * 5.f));
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
    box = sf::FloatRect({ box_x, center.y - box_height / 2.f }, { box_width, box_height });
    return true;
}

