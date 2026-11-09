#pragma once
#include "Enemy.h"

class Golem : public Enemy
{
private:
    enum class State { Idle, Walk, ThrowWindup, SmashWindup, Recover };
    State state = State::Idle;
    float stateTimer = 0.f;
    float throwCooldown = 1.f;
    float smashCooldown = 0.f;

    static constexpr float walkSpeed = 45.f;       // very slow
    static constexpr float sightRange = 800.f;     // starts attacking from this far
    static constexpr float throwMinRange = 260.f;  // throws rocks when the player is at least this far
    static constexpr float smashRange = 180.f;     // smashes the floor when the player is this close

    void throwRock(sf::Vector2f playerCenter, EnemyWorld& world);
    void smashGround(EnemyWorld& world);

public:
    Golem(float x, float y);

    void update(float deltaTime, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f playerCenter, EnemyWorld& world) override;
};