#include "../Classes/Phoenix.h"
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
