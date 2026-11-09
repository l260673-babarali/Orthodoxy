#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// Anything that flies: golem rocks, phoenix fireballs
class Projectile
{
private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float gravity;
    float lifetime;
    int damage;
    bool alive = true;

public:
    Projectile(sf::Vector2f center, sf::Vector2f size, sf::Vector2f startVelocity,
        float startGravity, int hitDamage, sf::Color color, float lifeSeconds);

    void update(float deltaTime, const std::vector<sf::RectangleShape>& solids);
    void draw(sf::RenderWindow& window);
    void kill();
    bool isAlive() const;
    int getDamage() const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getCenter() const;
};