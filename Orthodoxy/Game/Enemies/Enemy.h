#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../Combat/Projectile.h"
#include "../Combat/Spike.h"

// Lets enemies put rocks, fireballs and spikes into the game
struct EnemyWorld
{
    std::vector<Projectile>& projectiles;
    std::vector<Spike>& spikes;
};

class Enemy
{
protected:
    sf::RectangleShape shape;
    sf::Color baseColor;
    int health;
    bool alive = true;
    int lastHitStrikeId = -1;
    float hurtTimer = 0.f;
    float knockbackTimer = 0.f;
    float knockbackDir = 0.f;
    float knockbackSpeed = 250.f;
    float velocityY = 0.f;
    float facing = 1.f; // 1 = right, -1 = left

    void updateHurtFlash(float deltaTime);
    float consumeKnockback(float deltaTime);
    bool moveHorizontal(float moveX, const std::vector<sf::RectangleShape>& solids);
    void applyGravity(float deltaTime, const std::vector<sf::RectangleShape>& solids);

public:
    Enemy(sf::Vector2f position, sf::Vector2f size, sf::Color color, int startHealth);
    virtual ~Enemy() = default;

    virtual void update(float deltaTime, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f playerCenter, EnemyWorld& world) = 0;
    virtual void draw(sf::RenderWindow& window);
    virtual void takeDamage(int amount, int strikeId, float fromDirection);

    // Does just touching this enemy hurt the player?
    virtual bool hurtsOnContact() const { return true; }
    // Is the enemy hitting something right now (claws, explosion)? Fills 'box' if yes.
    virtual bool getAttackHitbox(sf::FloatRect&) const { return false; }

    bool isAlive() const;
    int getLastHitStrikeId() const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getCenter() const;
};