
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Level
{
private:
    sf::Texture backgroundTexture;
    sf::Sprite background;
    bool backgroundLoaded = false;

    std::vector<sf::RectangleShape> platforms;

    void addSolid(float x, float y, float w, float h);

public:
    Level();

    void draw(sf::RenderWindow& window);

    const std::vector<sf::RectangleShape>& getPlatforms() const;

    float getWidth() const;
    float getTop() const;
};
