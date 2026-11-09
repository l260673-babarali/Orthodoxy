#pragma once
#include "Enemy.h"

class Phoenix : public Enemy
{
private:
    enum class State { Hover, BreathWindup, Breathing, ClawWindup, ClawStrike, Recover };
    State state = State::Hover;
    float stateTimer = 0.f;
    float attackCooldown = 1.5f;
    float breathShotTimer = 0.f;
    float breathAngle = 0.f;
    sf::Vector2f lungeDir = { 1.f, 0.f };

    static constexpr float hoverSpeed = 170.f;
    static constexpr float hoverDistance = 300.f;  // how far away it likes to stay
    static constexpr float hoverHeight = 130.f;    // how high above the player
    static constexpr float clawRange = 220.f;      // closer than this = claws
    static constexpr float clawLungeSpeed = 420.f;

    void hoverNear(sf::Vector2f playerCenter, float deltaTime);

public:
    Phoenix(float x, float y);

    void update(float deltaTime, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f playerCenter, EnemyWorld& world) override;
    bool hurtsOnContact() const override { return false; }
    bool getAttackHitbox(sf::FloatRect& box) const override;
};