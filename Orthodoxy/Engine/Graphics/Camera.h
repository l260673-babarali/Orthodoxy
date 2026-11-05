#pragma once
#include <SFML/Graphics.hpp>

class Camera
{
private:
    sf::View view;

public:
    Camera();

    void follow(sf::Vector2f target, float worldWidth);
    void apply(sf::RenderWindow& window);
};