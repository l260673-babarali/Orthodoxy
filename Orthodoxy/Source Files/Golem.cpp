#include "../Classes/Golem.h"

namespace
{
    const sf::Color stoneColor(110, 110, 120);
    const sf::Color throwColor(210, 160, 60);   // golem turns yellow-brown before throwing
    const sf::Color smashColor(230, 60, 60);    // golem turns red before smashing
}

Golem::Golem(float x, float y)
    : Enemy({ x, y }, { 70.f, 90.f }, stoneColor, 6)
{
    knockbackSpeed = 60.f; // heavy, barely gets pushed
}

void Golem::update(float deltaTime, const std::vector<sf::RectangleShape>& solids,
    sf::Vector2f playerCenter, EnemyWorld& world)
{
    if (!alive)
        return;

    updateHurtFlash(deltaTime);

    if (throwCooldown > 0.f) throwCooldown -= deltaTime;
    if (smashCooldown > 0.f) smashCooldown -= deltaTime;
    stateTimer += deltaTime;

    moveHorizontal(consumeKnockback(deltaTime), solids);

    float dx = playerCenter.x - getCenter().x;
    float dist = std::abs(dx);

    switch (state)
    {
    case State::Idle:
    case State::Walk:
        baseColor = stoneColor;
        if (dist > sightRange)
        {
            state = State::Idle;
            break;
        }

        facing = (dx >= 0.f) ? 1.f : -1.f;

        if (dist <= smashRange && smashCooldown <= 0.f)
        {
            state = State::SmashWindup;
            stateTimer = 0.f;
        }
        else if (dist >= throwMinRange && throwCooldown <= 0.f)
        {
            state = State::ThrowWindup;
            stateTimer = 0.f;
        }
        else if (dist > smashRange * 0.6f)
        {
            // Slowly walk toward the player
            state = State::Walk;
            moveHorizontal(facing * walkSpeed * deltaTime, solids);
        }
        break;

    case State::ThrowWindup:
        baseColor = throwColor;
        facing = (dx >= 0.f) ? 1.f : -1.f;
        if (stateTimer >= 0.6f)
        {
            throwRock(playerCenter, world);
            throwCooldown = 2.5f;
            state = State::Recover;
            stateTimer = 0.f;
        }
        break;

    case State::SmashWindup:
        baseColor = smashColor;
        if (stateTimer >= 0.8f)
        {
            smashGround(world);
            smashCooldown = 3.f;
            state = State::Recover;
            stateTimer = 0.f;
        }
        break;

    case State::Recover:
        baseColor = stoneColor;
        if (stateTimer >= 0.6f)
        {
            state = State::Idle;
            stateTimer = 0.f;
        }
        break;
    }

    applyGravity(deltaTime, solids);
}

// Throws a rock in a curve (an arc) that lands where the player is standing now
void Golem::throwRock(sf::Vector2f target, EnemyWorld& world)
{
    sf::Vector2f start(getCenter().x + facing * 30.f, shape.getPosition().y + 10.f);
    const float rockGravity = 900.f;

    float dx = target.x - start.x;
    float dy = target.y - start.y;

    // Far away = longer flight time = higher arc
    float flightTime = std::clamp(std::abs(dx) / 450.f, 0.7f, 1.5f);
    float vx = dx / flightTime;
    float vy = (dy - 0.5f * rockGravity * flightTime * flightTime) / flightTime;

    world.projectiles.push_back(
        Projectile(start, { 36.f, 36.f }, { vx, vy }, rockGravity, 1,
            sf::Color(120, 110, 100), 4.f));
}

// Stone spikes pop out on both sides, one after another, moving outward
void Golem::smashGround(EnemyWorld& world)
{
    float groundY = shape.getPosition().y + shape.getSize().y;
    float centerX = getCenter().x;

    for (int i = 1; i <= 6; i++)
    {
        float offset = i * 75.f + 20.f;
        float delay = i * 0.09f;
        world.spikes.push_back(Spike(centerX - offset, groundY, delay));
        world.spikes.push_back(Spike(centerX + offset, groundY, delay));
    }
}