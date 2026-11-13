#include "../Classes/Projectile.h"
#include <cmath>
#include <iostream>

namespace
{
    // One shared copy of the rock picture for every rock in the game
    sf::Texture& rockTexture()
    {
        static sf::Texture texture;
        static bool tried = false;
        if (!tried)
        {
            tried = true;
            if (!texture.loadFromFile("Assets/Sprites/Enemies/Golem/rock_sheet.png"))
                std::cout << "Could not load rock_sheet.png\n";
        }
        return texture;
    }
    constexpr int rock_cell = 128;      // one frame in rock_sheet.png
    constexpr int rock_frames = 5;
    constexpr float rock_scale = 0.65f;
}

Projectile::Projectile(sf::Vector2f center, sf::Vector2f size, sf::Vector2f start_velocity,
    float start_gravity, int hit_damage, sf::Color color, float life_seconds, bool rock)
    : velocity(start_velocity), gravity(start_gravity), lifetime(life_seconds), damage(hit_damage),
    is_rock(rock)
{
    shape.setSize(size);
    shape.setOrigin(size / 2.f);   // position = the middle of the projectile
    shape.setPosition(center);
    shape.setFillColor(color);
}

void Projectile::update(float delta_time, const std::vector<sf::RectangleShape>& solids)
{
    if (!alive)
        return;

    age += delta_time;
    velocity.y += gravity * delta_time;   // gravity slowly bends the path downward
    shape.move(velocity * delta_time);

    lifetime -= delta_time;
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
    if (!alive)
        return;

    const sf::Texture& texture = rockTexture();
    if (is_rock && texture.getSize().x > 0)
    {
        // Rocks use the animated picture
        sf::Sprite sprite(texture);
        int frame = static_cast<int>(age * 14.f) % rock_frames;   // 14 pictures per second
        sprite.setTextureRect(sf::IntRect({ frame * rock_cell, 0 }, { rock_cell, rock_cell }));
        sprite.setOrigin({ rock_cell / 2.f, rock_cell / 2.f });
        sprite.setPosition(shape.getPosition());

        // The picture flies to the right with its purple trail behind it.
        // Turn it to point where the rock is going (flip it if going left).
        float angle;
        if (velocity.x >= 0.f)
        {
            sprite.setScale({ rock_scale, rock_scale });
            angle = std::atan2(velocity.y, velocity.x);
        }
        else
        {
            sprite.setScale({ -rock_scale, rock_scale });
            angle = std::atan2(-velocity.y, -velocity.x);
        }
        sprite.setRotation(sf::radians(angle));
        window.draw(sprite);
    }
    else
    {
        // Fireballs (and a rock with no picture) are plain boxes
        window.draw(shape);
    }
}

void Projectile::kill() { alive = false; }
bool Projectile::isAlive() const { return alive; }
int Projectile::getDamage() const { return damage; }
sf::FloatRect Projectile::getBounds() const { return shape.getGlobalBounds(); }
sf::Vector2f Projectile::getCenter() const { return shape.getPosition(); }
