#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
class Level {
    std::vector<sf::Texture> pictures;
    std::vector<sf::RectangleShape> platforms;
    bool cathedral = false;
    void addSolid(float x, float y, float width, float height);
public:
    Level();
    void reset();
    void draw(sf::RenderWindow& window);
    const std::vector<sf::RectangleShape>& getPlatforms() const;
    float getWidth() const;
    float getTop() const;
    float getHeight() const;
    bool isCathedral() const;
    void enterCathedral();
};
