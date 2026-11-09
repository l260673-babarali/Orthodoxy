#include "BurstSpirit.h"

namespace
{
    const sf::Color spiritColor(170, 120, 255);
}

BurstSpirit::BurstSpirit(float x, float y)
    : Enemy({ x, y }, { 30.f, 30.f }, spiritColor, 2)
{
}

void BurstSpirit::update(float deltaTime, const std::vector<sf::RectangleShape>&,
    sf::Vector2f playerCenter, EnemyWorld&)
{
    if (!alive)
        return;

    updateHurtFlash(deltaTime);
    bobTimer += deltaTime;
    stateTimer += deltaTime;
    shape.move({ consumeKnockback(deltaTime), 0.f });

    // ---- Is the player standing still? ----
    float playerSpeed = 1000.f; // pretend "moving" on the very first frame
    if (hasLastPlayerPos && deltaTime > 0.f)
    {
        sf::Vector2f moved = playerCenter - lastPlayerCenter;
        playerSpeed = std::sqrt(moved.x * moved.x + moved.y * moved.y) / deltaTime;
    }
    lastPlayerCenter = playerCenter;
    hasLastPlayerPos = true;

    if (playerSpeed < 20.f)
        stillTimer += deltaTime;
    else
        stillTimer = 0.f; // moving resets the countdown

    // ---- Distance to the player ----
    sf::Vector2f toPlayer = playerCenter - getCenter();
    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (dist <= sightRange)
        noticed = true;

    switch (state)
    {
    case State::Chase:
        baseColor = spiritColor;
        if (noticed)
        {
            if (dist > 40.f)
                shape.move(toPlayer / dist * chaseSpeed * deltaTime);

            // Gentle floating up and down
            shape.move({ 0.f, std::sin(bobTimer * 6.f) * 25.f * deltaTime });

            // Close AND player stopped for a bit = start the countdown
            if (dist <= triggerRange && stillTimer >= stillTimeNeeded)
            {
                state = State::Fuse;
                stateTimer = 0.f;
            }
        }
        break;

    case State::Fuse:
        // Flash red/white. The spirit stops and waits, so you can run away or kill it.
        baseColor = (static_cast<int>(stateTimer * 12.f) % 2 == 0)
            ? sf::Color(255, 80, 80) : sf::Color::White;
        if (stateTimer >= fuseTime)
        {
            state = State::Exploding;
            stateTimer = 0.f;
        }
        break;

    case State::Exploding:
        baseColor = sf::Color(255, 160, 50);
        if (stateTimer >= explosionTime)
            alive = false;
        break;
    }
}

// Can't be killed during the boom itself
void BurstSpirit::takeDamage(int amount, int strikeId, float fromDirection)
{
    if (state == State::Exploding)
        return;
    Enemy::takeDamage(amount, strikeId, fromDirection);
}

bool BurstSpirit::getAttackHitbox(sf::FloatRect& box) const
{
    if (state != State::Exploding)
        return false;

    sf::Vector2f c = getCenter();
    box = sf::FloatRect({ c.x - explosionRadius, c.y - explosionRadius },
        { explosionRadius * 2.f, explosionRadius * 2.f });
    return true;
}