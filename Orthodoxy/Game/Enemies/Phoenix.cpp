#include "Phoenix.h"

namespace
{
    const sf::Color normalColor(255, 150, 40);
    const sf::Color breathColor(255, 60, 0);     // red-orange = fire is coming
    const sf::Color clawColor(255, 230, 60);     // yellow = claws are coming
}

Phoenix::Phoenix(float x, float y)
    : Enemy({ x, y }, { 44.f, 32.f }, normalColor, 3)
{
}

// Fly to a spot in front of the player, a bit above them
void Phoenix::hoverNear(sf::Vector2f playerCenter, float deltaTime)
{
    sf::Vector2f center = getCenter();
    float side = (center.x < playerCenter.x) ? -1.f : 1.f;

    float targetX = playerCenter.x + side * hoverDistance;
    float targetY = std::min(playerCenter.y - hoverHeight, 440.f); // stay above the floor

    sf::Vector2f toTarget(targetX - center.x, targetY - center.y);
    float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

    if (dist > 4.f)
    {
        float step = std::min(hoverSpeed * deltaTime, dist);
        shape.move(toTarget / dist * step);
    }
}

void Phoenix::update(float deltaTime, const std::vector<sf::RectangleShape>&,
    sf::Vector2f playerCenter, EnemyWorld& world)
{
    if (!alive)
        return;

    updateHurtFlash(deltaTime);
    shape.move({ consumeKnockback(deltaTime), 0.f });
    stateTimer += deltaTime;

    sf::Vector2f center = getCenter();
    sf::Vector2f toPlayer = playerCenter - center;

    switch (state)
    {
    case State::Hover:
        baseColor = normalColor;
        facing = (toPlayer.x >= 0.f) ? 1.f : -1.f;
        hoverNear(playerCenter, deltaTime);

        attackCooldown -= deltaTime;
        if (attackCooldown <= 0.f)
        {
            // Close = claws, far = fire breath
            bool close = std::abs(toPlayer.x) < clawRange && std::abs(toPlayer.y) < 160.f;
            state = close ? State::ClawWindup : State::BreathWindup;
            stateTimer = 0.f;
        }
        break;

    case State::BreathWindup:
        baseColor = breathColor;
        facing = (toPlayer.x >= 0.f) ? 1.f : -1.f;
        if (stateTimer >= 0.55f)
        {
            // Aim once, now. If you move, the fire misses!
            breathAngle = std::atan2(toPlayer.y, toPlayer.x);
            breathShotTimer = 0.f;
            state = State::Breathing;
            stateTimer = 0.f;
        }
        break;

    case State::Breathing:
        baseColor = breathColor;
        breathShotTimer -= deltaTime;
        if (breathShotTimer <= 0.f)
        {
            breathShotTimer = 0.07f;
            // The fire wiggles a little up and down
            float angle = breathAngle + std::sin(stateTimer * 14.f) * 0.15f;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));

            world.projectiles.push_back(
                Projectile(center + dir * 26.f, { 16.f, 16.f }, dir * 430.f, 0.f, 1,
                    sf::Color(255, 140, 30), 0.7f));
        }
        if (stateTimer >= 1.0f)
        {
            attackCooldown = 2.f;
            state = State::Recover;
            stateTimer = 0.f;
        }
        break;

    case State::ClawWindup:
        baseColor = clawColor;
        facing = (toPlayer.x >= 0.f) ? 1.f : -1.f;
        if (stateTimer >= 0.4f)
        {
            float len = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
            if (len > 0.f)
                lungeDir = toPlayer / len;
            else
                lungeDir = { facing, 0.f };

            facing = (lungeDir.x >= 0.f) ? 1.f : -1.f;
            state = State::ClawStrike;
            stateTimer = 0.f;
        }
        break;

    case State::ClawStrike:
        baseColor = clawColor;
        shape.move(lungeDir * clawLungeSpeed * deltaTime); // swoops at the player
        if (stateTimer >= 0.25f)
        {
            attackCooldown = 1.2f;
            state = State::Recover;
            stateTimer = 0.f;
        }
        break;

    case State::Recover:
        baseColor = normalColor;
        hoverNear(playerCenter, deltaTime);
        if (stateTimer >= 0.7f)
        {
            state = State::Hover;
            stateTimer = 0.f;
        }
        break;
    }
}

// The claws only hurt during the swoop
bool Phoenix::getAttackHitbox(sf::FloatRect& box) const
{
    if (state != State::ClawStrike)
        return false;

    sf::Vector2f c = getCenter();
    const float w = 90.f;
    const float h = 80.f;
    float x = (facing > 0.f) ? c.x : c.x - w;

    box = sf::FloatRect({ x, c.y - h / 2.f }, { w, h });
    return true;
}