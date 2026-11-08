#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Enemy
{
private:
    sf::RectangleShape shape;

    float speed = 80.f;
    float direction = 1.f;          // 1 = right, -1 = left
    float velocityY = 0.f;

    float patrolLeft;
    float patrolRight;

    int health = 3;
    bool alive = true;

    int lastHitStrikeId = -1;       // remembers which player swing hit us last

    float hurtTimer = 0.f;          // flashes white after being hit
    float knockbackTimer = 0.f;
    float knockbackDir = 0.f;

public:
    Enemy(float x, float y, float patrolLeft, float patrolRight);

    void update(float deltaTime, const std::vector<sf::RectangleShape>& solids);
    void draw(sf::RenderWindow& window);

    void takeDamage(int amount, int strikeId, float fromDirection);

    bool isAlive() const;
    int getLastHitStrikeId() const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getCenter() const;
};