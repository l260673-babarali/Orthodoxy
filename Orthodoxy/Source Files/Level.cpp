
#include "../Classes/Level.h"

void Level::addSolid(float x, float y, float w, float h)
{
    sf::RectangleShape solid({ w, h });
    solid.setPosition({ x, y });

    // Temporary visible collision boxes
    solid.setFillColor(sf::Color(255, 0, 0, 100));
    solid.setOutlineColor(sf::Color::Red);
    solid.setOutlineThickness(1.f);

    platforms.push_back(solid);
}

Level::Level()
    : background(backgroundTexture)
{
    backgroundLoaded =
        backgroundTexture.loadFromFile("Assets/Sprites/monastery_entrance.png");

    if (backgroundLoaded)
        background.setPosition({ 0.f, 0.f });

    // Main floor
    addSolid(0.f, 686.f, 1360.f, 82.f);

    // Left platforms
    addSolid(0.f, 360.f, 128.f, 18.f);
    addSolid(190.f, 360.f, 130.f, 18.f);

    // Central upper platform
    addSolid(400.f, 278.f, 192.f, 20.f);

    // Right platforms
    addSolid(696.f, 487.f, 154.f, 18.f);
    addSolid(951.f, 468.f, 156.f, 18.f);
    addSolid(1171.f, 609.f, 189.f, 18.f);

    // Central staircase
    addSolid(652.f, 444.f, 90.f, 18.f);

    // Starting platform
    addSolid(78.f, 625.f, 110.f, 16.f);
}

void Level::draw(sf::RenderWindow& window)
{
    if (backgroundLoaded)
        window.draw(background);

    for (const auto& platform : platforms)
        window.draw(platform);
}

const std::vector<sf::RectangleShape>&
Level::getPlatforms() const
{
    return platforms;
}

float Level::getWidth() const
{
    return 1360.f;
}

float Level::getTop() const
{
    return 0.f;
}
