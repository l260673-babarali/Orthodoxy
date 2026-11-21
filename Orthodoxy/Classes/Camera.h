#pragma once
#include <SFML/Graphics.hpp>

// Follows the player but never shows anything outside the level
class Camera
{
private:
    sf::View view;

public:
    Camera();

    // Height of the world that is visible (width follows the 16:9 window). Bigger = more of the picture, smaller knight.
    void setViewHeight(float height);
    void follow(sf::Vector2f target, float world_width, float world_top, float world_bottom);
    void apply(sf::RenderWindow& window);
    sf::Vector2f getCenter() const;
    const sf::View& getView() const;
};
