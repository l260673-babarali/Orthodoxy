#pragma once
#include <SFML/Graphics.hpp>

// Follows the player but never shows anything outside the level
class Camera
{
private:
    sf::View view;

public:
    Camera();

    void follow(sf::Vector2f target, float world_width, float world_top, float world_bottom);
    void apply(sf::RenderWindow& window);
    sf::Vector2f getCenter() const;
};
