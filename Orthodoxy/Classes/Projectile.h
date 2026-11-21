#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// Anything that flies: golem rocks, phoenix fireballs
class Projectile
{
private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float gravity;         // 0 = flies straight (fireball), above 0 = falls in an arc (rock)
    float lifetime;        // seconds left before it disappears on its own
    int damage;
    bool alive = true;
    bool is_rock = false;   // true = draw the rock picture instead of a plain box
    float age = 0.f;       // used to animate the rock

public:
    Projectile(sf::Vector2f center, sf::Vector2f size, sf::Vector2f start_velocity,
        float start_gravity, int hit_damage, sf::Color color, float life_seconds,
        bool rock = false);
    void update(float delta_time, const std::vector<sf::RectangleShape>& solids);
    void draw(sf::RenderWindow& window);
    void kill();
    bool isAlive() const;
    int getDamage() const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getCenter() const;
};
