#include "../Classes/Golem.h"
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
