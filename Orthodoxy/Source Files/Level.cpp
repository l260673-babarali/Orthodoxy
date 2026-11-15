#include "../Classes/Level.h"
#include <iostream>
#include <string>

const float OLD_SCALE = 1.5f;
const float OLD_SCENE = 543.f * OLD_SCALE;
const float SCENE_WIDTH = 1360.f; // cathedral scene width
const float CONTINUATION_WIDTH = 1086.f;
const float HEIGHT = 1086.f;
const float EXTERIOR_WIDTH = 2172.f * OLD_SCALE + 3.f * CONTINUATION_WIDTH;

void Level::addSolid(float x, float y, float width, float height)
{
    sf::RectangleShape box({width, height});
    box.setPosition({x, y});
    box.setFillColor(sf::Color::Transparent);
    box.setOutlineColor(sf::Color::Green);
    box.setOutlineThickness(2.f);
    platforms.push_back(box);
}

Level::Level()
{
    pictures.resize(10);
    for (int i = 0; i < 4; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/scene"
                         + std::to_string(i + 1) + ".png";
        if (!pictures[i].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/continuation" + std::to_string(i + 1) + ".png";
        if (!pictures[i + 4].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/cathedral" + std::to_string(i + 1) + ".png";
        if (!pictures[i + 7].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 10; i++) pictures[i].setSmooth(false);

    reset();
}

void Level::reset()
{
    cathedral = false;
    platforms.clear();
    // Original Ashen Barrow platforms: preserved.
    addSolid(24 * OLD_SCALE, 275 * OLD_SCALE, 282 * OLD_SCALE, 12.f);
    addSolid(48 * OLD_SCALE, 499 * OLD_SCALE, 246 * OLD_SCALE, 12.f);
    addSolid(240 * OLD_SCALE, 418 * OLD_SCALE, 132 * OLD_SCALE, 12.f);
    addSolid(291 * OLD_SCALE, 592 * OLD_SCALE, 297 * OLD_SCALE, 12.f);
    addSolid(369 * OLD_SCALE, 353 * OLD_SCALE, 220 * OLD_SCALE, 12.f);
    addSolid(585 * OLD_SCALE, 569 * OLD_SCALE, 151 * OLD_SCALE, 12.f);
    addSolid(664 * OLD_SCALE, 298 * OLD_SCALE, 107 * OLD_SCALE, 12.f);
    addSolid(769 * OLD_SCALE, 484 * OLD_SCALE, 157 * OLD_SCALE, 12.f);
    addSolid(772 * OLD_SCALE, 301 * OLD_SCALE, 92 * OLD_SCALE, 12.f);
    addSolid(887 * OLD_SCALE, 324 * OLD_SCALE, 105 * OLD_SCALE, 12.f);
    addSolid(911 * OLD_SCALE, 579 * OLD_SCALE, 273 * OLD_SCALE, 12.f);
    addSolid(982 * OLD_SCALE, 397 * OLD_SCALE, 148 * OLD_SCALE, 12.f);
    addSolid(1143 * OLD_SCALE, 418 * OLD_SCALE, 90 * OLD_SCALE, 12.f);
    addSolid(1244 * OLD_SCALE, 369 * OLD_SCALE, 125 * OLD_SCALE, 12.f);
    addSolid(1331 * OLD_SCALE, 611 * OLD_SCALE, 277 * OLD_SCALE, 12.f);
    addSolid(1529 * OLD_SCALE, 374 * OLD_SCALE, 154 * OLD_SCALE, 12.f);
    addSolid(1609 * OLD_SCALE, 592 * OLD_SCALE, 538 * OLD_SCALE, 12.f);
    addSolid(1667 * OLD_SCALE, 270 * OLD_SCALE, 147 * OLD_SCALE, 12.f);
    addSolid(1694 * OLD_SCALE, 463 * OLD_SCALE, 238 * OLD_SCALE, 12.f);
    addSolid(1955 * OLD_SCALE, 402 * OLD_SCALE, 206 * OLD_SCALE, 12.f);
    // Distinct cemetery-and-waterfall continuation.d
    // Each raw pixel is multiplied by 1.5 for the game world.
    float start = 2172.f * OLD_SCALE;

  
    addSolid(start + 0.f * OLD_SCALE, 526.f * OLD_SCALE, 337.f * OLD_SCALE, 16.f);
    addSolid(start + 0.f * OLD_SCALE, 229.f * OLD_SCALE, 260.f * OLD_SCALE, 16.f);
    addSolid(start + 200.f * OLD_SCALE, 351.f * OLD_SCALE, 396.f * OLD_SCALE, 16.f);
    addSolid(start + 345.f * OLD_SCALE, 600.f * OLD_SCALE, 300.f * OLD_SCALE, 16.f);
    addSolid(start + 620.f * OLD_SCALE, 478.f * OLD_SCALE, 315.f * OLD_SCALE, 16.f);

    addSolid(start + 870.f * OLD_SCALE, 362.f * OLD_SCALE, 350.f * OLD_SCALE, 16.f);
    addSolid(start + 1150.f * OLD_SCALE, 633.f * OLD_SCALE, 160.f * OLD_SCALE, 16.f);
    addSolid(start + 1220.f * OLD_SCALE, 276.f * OLD_SCALE, 250.f * OLD_SCALE, 16.f);
    addSolid(start + 1275.f * OLD_SCALE, 542.f * OLD_SCALE, 252.f * OLD_SCALE, 16.f);

    addSolid(start + 1550.f * OLD_SCALE, 446.f * OLD_SCALE, 290.f * OLD_SCALE, 16.f);
    addSolid(start + 1700.f * OLD_SCALE, 336.f * OLD_SCALE, 268.f * OLD_SCALE, 16.f);
    addSolid(start + 1980.f * OLD_SCALE, 478.f * OLD_SCALE, 246.f * OLD_SCALE, 16.f);
    addSolid(start + 1990.f * OLD_SCALE, 211.f * OLD_SCALE, 252.f * OLD_SCALE, 16.f);

}

void Level::enterCathedral()
{
    cathedral = true;
    platforms.clear();
    for (int i = 0; i < 3; i++)
    {
        float x = i * SCENE_WIDTH;
        addSolid(x, 900.f, SCENE_WIDTH, 40.f);
        addSolid(x + 160.f, 750.f, 250.f, 16.f);
        addSolid(x + 520.f, 630.f, 230.f, 16.f);
        addSolid(x + 900.f, 700.f, 250.f, 16.f);
    }
}

bool Level::isCathedral() const { return cathedral; }

void Level::draw(sf::RenderWindow& window)
{
    if (!cathedral)
    {
        for (int i = 0; i < 4; i++)
        {
            sf::Sprite s(pictures[i]);
            s.setScale({OLD_SCALE, OLD_SCALE});
            s.setPosition({i * OLD_SCENE, 0.f});
            window.draw(s);
        }
        for (int i = 0; i < 3; i++)
        {
            sf::Sprite s(pictures[i + 4]);
            s.setPosition({2172.f * OLD_SCALE + i * CONTINUATION_WIDTH, 0.f});
            s.setScale({CONTINUATION_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
            window.draw(s);
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            sf::Sprite s(pictures[i + 7]);
            s.setPosition({i * SCENE_WIDTH, 0.f});
            s.setScale({SCENE_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
            window.draw(s);
        }
    }
    // Change this to true only when aligning collision rectangles.
    bool showPlatforms = true;
    if (showPlatforms)
    {
        for (int i = 0; i < (int)platforms.size(); i++)
            window.draw(platforms[i]);
    }
}

const std::vector<sf::RectangleShape>& Level::getPlatforms() const 
{ return platforms; }
float Level::getWidth() const 
{ return cathedral ? 3.f * SCENE_WIDTH : EXTERIOR_WIDTH; }
float Level::getTop() const 
{ return 0.f; }
float Level::getHeight() const 
{ return HEIGHT; }
