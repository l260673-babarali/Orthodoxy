#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// The map: background picture plus all the platforms the player and enemies stand on
class Level
{
private:
    sf::Texture background_texture;
    sf::Sprite background;
    bool background_loaded = false;

    std::vector<sf::RectangleShape> platforms;

    void addSolid(float x, float y, float width, float height);

public:
    Level();

    void draw(sf::RenderWindow& window);

    const std::vector<sf::RectangleShape>& getPlatforms() const;

    float getWidth() const;
    float getTop() const;
    float getHeight() const;
};
