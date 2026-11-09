#pragma once
#include "Enemy.h"

class BurstSpirit : public Enemy
{
private:
    enum class State { Chase, Fuse, Exploding };
    State state = State::Chase;
    float stateTimer = 0.f;
    float bobTimer = 0.f;
    bool noticed = false;

    // Used to find out if the player stopped moving
    sf::Vector2f lastPlayerCenter;
    bool hasLastPlayerPos = false;
    float stillTimer = 0.f;

    static constexpr float chaseSpeed = 150.f;       // player walks at 200, so you can outrun it
    static constexpr float sightRange = 600.f;
    static constexpr float triggerRange = 130.f;     // must be this close to start the countdown
    static constexpr float stillTimeNeeded = 0.5f;   // player must stand still this long
    static constexpr float fuseTime = 0.6f;          // flashing before the boom
    static constexpr float explosionTime = 0.2f;
    static constexpr float explosionRadius = 110.f;

public:
    BurstSpirit(float x, float y);

    void update(float deltaTime, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f playerCenter, EnemyWorld& world) override;
    void takeDamage(int amount, int strikeId, float fromDirection) override;
    bool hurtsOnContact() const override { return false; }
    bool getAttackHitbox(sf::FloatRect& box) const override;
};