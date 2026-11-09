#include "Level.h"

void Level::addSolid(float x, float y, float w, float h, sf::Color color)
{
    sf::RectangleShape rect({ w, h });
    rect.setPosition({ x, y });
    rect.setFillColor(color);
    platforms.push_back(rect);
}

Level::Level()
{
    const sf::Color groundColor(70, 80, 90);
    const sf::Color platformColor(110, 100, 90);

    // Ground (now a normal solid)
    addSolid(0.f, 520.f, 2400.f, 80.f, groundColor);

    // Floating platforms
    addSolid(220.f, 420.f, 160.f, 20.f, platformColor);
    addSolid(420.f, 340.f, 150.f, 20.f, platformColor);
    addSolid(620.f, 420.f, 130.f, 20.f, platformColor);
    addSolid(850.f, 420.f, 180.f, 20.f, platformColor);
    addSolid(1150.f, 300.f, 160.f, 20.f, platformColor);
    addSolid(1550.f, 420.f, 200.f, 20.f, platformColor);

    // Solid block to test side collision
    addSolid(1850.f, 440.f, 80.f, 80.f, platformColor);
    // Solid block to test side collision
    addSolid(1850.f, 440.f, 80.f, 80.f, platformColor);

    // Huge wall: can't be cleared with jumps alone
    addSolid(2200.f, -800.f, 60.f, 1320.f, sf::Color(50, 50, 60));
    // Goal ledge on top of the wall
    addSolid(2260.f, -800.f, 140.f, 20.f, platformColor);

}

void Level::draw(sf::RenderWindow& window)
{
    for (const auto& platform : platforms)
    {
        window.draw(platform);
    }
}

const std::vector<sf::RectangleShape>& Level::getPlatforms() const
{
    return platforms;
}

float Level::getWidth() const
{
    return 2400.f;
}

float Level::getTop() const
{
    return -900.f;
}