#include "../Classes/BurstSpirit.h"

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
