#include "../Classes/Projectile.h"

Projectile::Projectile(sf::Vector2f center, sf::Vector2f size, sf::Vector2f startVelocity,
    float startGravity, int hitDamage, sf::Color color, float lifeSeconds)
    : velocity(startVelocity), gravity(startGravity), lifetime(lifeSeconds), damage(hitDamage)
{
    shape.setSize(size);
    shape.setOrigin(size / 2.f); // position = the middle of the projectile
    shape.setPosition(center);
    shape.setFillColor(color);
}

void Projectile::update(float deltaTime, const std::vector<sf::RectangleShape>& solids)
{
    if (!alive)
        return;

    velocity.y += gravity * deltaTime;
    shape.move(velocity * deltaTime);

    lifetime -= deltaTime;
    if (lifetime <= 0.f)
    {
        alive = false;
        return;
    }

    // Break when it hits the ground, a wall or a platform
    for (const auto& solid : solids)
    {
        if (shape.getGlobalBounds().findIntersection(solid.getGlobalBounds()))
        {
            alive = false;
            return;
        }
    }
}

void Projectile::draw(sf::RenderWindow& window)
{
    if (alive)
        window.draw(shape);
}

void Projectile::kill() { alive = false; }
bool Projectile::isAlive() const { return alive; }
int Projectile::getDamage() const { return damage; }
sf::FloatRect Projectile::getBounds() const { return shape.getGlobalBounds(); }
sf::Vector2f Projectile::getCenter() const { return shape.getPosition(); }