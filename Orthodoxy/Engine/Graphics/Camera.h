#pragma once
#include <SFML/Graphics.hpp>

class Camera
{
private:
    sf::View view;

public:
    Camera();

    void follow(sf::Vector2f target, float worldWidth, float worldTop);
    void apply(sf::RenderWindow& window);
};