#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Level
{
private:
    sf::RectangleShape ground;
    std::vector<sf::RectangleShape> platforms;

public:
    Level();

    void draw(sf::RenderWindow& window);

    const std::vector<sf::RectangleShape>& getPlatforms() const;

    float getWidth() const;
};