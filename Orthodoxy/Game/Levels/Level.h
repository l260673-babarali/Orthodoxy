#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Level
{
private:
    // Everything solid: ground, platforms, blocks
    std::vector<sf::RectangleShape> platforms;

    void addSolid(float x, float y, float w, float h, sf::Color color);

public:
    Level();
    void draw(sf::RenderWindow& window);
    const std::vector<sf::RectangleShape>& getPlatforms() const;
    float getWidth() const;
};