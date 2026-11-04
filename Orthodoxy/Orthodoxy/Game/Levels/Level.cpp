#include "Level.h"

Level::Level()
{
    // Main ground
    ground.setSize({ 2400.f, 80.f });
    ground.setPosition({ 0.f, 520.f });
    ground.setFillColor(sf::Color(70, 80, 90));

    // Platform 1
    sf::RectangleShape platform1({ 160.f, 20.f });
    platform1.setPosition({ 220.f, 450.f });
    platform1.setFillColor(sf::Color(110, 100, 90));

    // Platform 2
    sf::RectangleShape platform2({ 150.f, 20.f });
    platform2.setPosition({ 420.f, 380.f });
    platform2.setFillColor(sf::Color(110, 100, 90));

    // Platform 3
    sf::RectangleShape platform3({ 130.f, 20.f });
    platform3.setPosition({ 620.f, 450.f });
    platform3.setFillColor(sf::Color(110, 100, 90));

    // Platform 4
    sf::RectangleShape platform4({ 180.f, 20.f });
    platform4.setPosition({ 850.f, 450.f });
    platform4.setFillColor(sf::Color(110, 100, 90));

    // Platform 5
    sf::RectangleShape platform5({ 160.f, 20.f });
    platform5.setPosition({ 1150.f, 370.f });
    platform5.setFillColor(sf::Color(110, 100, 90));

    // Platform 6
    sf::RectangleShape platform6({ 200.f, 20.f });
    platform6.setPosition({ 1550.f, 450.f });
    platform6.setFillColor(sf::Color(110, 100, 90));

    platforms.push_back(platform1);
    platforms.push_back(platform2);
    platforms.push_back(platform3);
    platforms.push_back(platform4);
    platforms.push_back(platform5);
    platforms.push_back(platform6);
}

void Level::draw(sf::RenderWindow& window)
{
    window.draw(ground);

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