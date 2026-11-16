#include "../Classes/Level.h"
#include <iostream>
#include <string>

const float OLD_SCALE = 1.5f;
const float OLD_SCENE = 543.f * OLD_SCALE;
const float SCENE_WIDTH = 1360.f; // cathedral scene width
const float CONTINUATION_WIDTH = 1086.f;
const float HEIGHT = 1086.f;
const float EXTERIOR_WIDTH = 2172.f * OLD_SCALE + 3.f * CONTINUATION_WIDTH;
const float RUINS_SCENE_WIDTH = 1940.f; // each red-moon-ruins picture (scene5, 6, 7) is stretched to this width
const float RUINS_WIDTH = 3.f * RUINS_SCENE_WIDTH;

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
    pictures.resize(13);
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
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/scene" + std::to_string(i + 5) + ".png";
        if (!pictures[i + 10].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 13; i++) pictures[i].setSmooth(false);

    reset();
}

void Level::reset()
{
    setRegion(Region::AshenBarrow);
}

void Level::setRegion(Region newRegion)
{
    region = newRegion;
    platforms.clear();
    if (region == Region::AshenBarrow)       buildAshenBarrow();
    else if (region == Region::RedMoonRuins) buildRedMoonRuins();
    else                                     buildCathedral();
}

void Level::buildAshenBarrow()
{
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

// ---------------------------------------------------------------------------
// Region 2: Red Moon Ruins. Three pictures side by side (scene5, scene6, scene7),
// each stretched to 1940 x 1086. All numbers below are positions INSIDE one picture;
// "scene" is 0, 1 or 2 and moves the platform to the right picture.
// The numbers were estimated from the pictures: switch showPlatforms on in
// Level::draw (it is on by default) and click in the game to print the real
// coordinates, then nudge any platform that is a few pixels off.
// ---------------------------------------------------------------------------
void Level::buildRedMoonRuins()
{
    auto add = [this](int scene, float x, float y, float w)
    {
        addSolid(scene * RUINS_SCENE_WIDTH + x, y, w, 14.f);
    };

    // ----- scene5 (first picture) -----
    add(0, 0.f, 878.f, 745.f);     // left floor
    add(0, 0.f, 705.f, 210.f);     // ledge above the stairs
    add(0, 205.f, 725.f, 45.f);    // stairs going down
    add(0, 245.f, 760.f, 45.f);
    add(0, 285.f, 795.f, 45.f);
    add(0, 325.f, 830.f, 45.f);
    add(0, 245.f, 595.f, 280.f);   // torch ledge
    add(0, 215.f, 484.f, 110.f);   // wooden swing
    add(0, 0.f, 322.f, 385.f);     // top-left ledge + plank
    add(0, 510.f, 618.f, 330.f);   // rope bridge
    add(0, 645.f, 415.f, 85.f);    // hanging swing
    add(0, 715.f, 512.f, 415.f);   // big arch top
    add(0, 1130.f, 540.f, 45.f);   // stairs going down to the right
    add(0, 1170.f, 562.f, 45.f);
    add(0, 1210.f, 585.f, 45.f);
    add(0, 1100.f, 625.f, 220.f);
    add(0, 1290.f, 575.f, 650.f);  // long right platform (continues into scene6)
    add(0, 1340.f, 378.f, 85.f);   // broken pillar top
    add(0, 1465.f, 275.f, 435.f);  // high right platform
    add(0, 1500.f, 772.f, 60.f);   // short pillar
    add(0, 610.f, 995.f, 790.f);   // bottom middle ground
    add(0, 1400.f, 905.f, 540.f);  // bottom right ground

    // ----- scene6 (second picture) -----
    add(1, 0.f, 495.f, 440.f);     // portal ledge
    add(1, 440.f, 530.f, 45.f);    // stairs going down
    add(1, 480.f, 565.f, 45.f);
    add(1, 500.f, 608.f, 190.f);
    add(1, 695.f, 660.f, 40.f);
    add(1, 705.f, 705.f, 275.f);   // torch table
    add(1, 585.f, 548.f, 225.f);   // small upper platform
    add(1, 940.f, 575.f, 180.f);
    add(1, 1120.f, 565.f, 295.f);  // brazier platform
    add(1, 1260.f, 775.f, 240.f);
    add(1, 1480.f, 632.f, 460.f);  // right platform with the dead tree
    add(1, 0.f, 748.f, 230.f);     // lower-left ledge
    add(1, 230.f, 775.f, 40.f);    // stairs going down
    add(1, 270.f, 805.f, 40.f);
    add(1, 310.f, 835.f, 40.f);
    add(1, 300.f, 878.f, 250.f);
    add(1, 550.f, 905.f, 50.f);
    add(1, 0.f, 928.f, 300.f);     // ground, left
    add(1, 600.f, 938.f, 590.f);   // ground, middle
    add(1, 1190.f, 972.f, 310.f);
    add(1, 1500.f, 925.f, 440.f);  // ground, right

    // ----- scene7 (third picture, the exit portal is on the right) -----
    add(2, 0.f, 688.f, 460.f);     // left stone ledge
    add(2, 460.f, 678.f, 140.f);   // wooden bridge
    add(2, 595.f, 672.f, 50.f);    // stairs going up
    add(2, 640.f, 645.f, 50.f);
    add(2, 685.f, 618.f, 50.f);
    add(2, 735.f, 590.f, 50.f);
    add(2, 780.f, 565.f, 50.f);
    add(2, 790.f, 552.f, 330.f);   // banner platform
    add(2, 1120.f, 555.f, 170.f);  // bridge
    add(2, 1290.f, 585.f, 240.f);
    add(2, 1520.f, 562.f, 45.f);
    add(2, 1560.f, 545.f, 380.f);  // portal ledge
    add(2, 1410.f, 735.f, 230.f);  // lower right ledge
    add(2, 1600.f, 722.f, 50.f);   // big stairs going up to the portal
    add(2, 1650.f, 692.f, 50.f);
    add(2, 1700.f, 662.f, 50.f);
    add(2, 1750.f, 632.f, 50.f);
    add(2, 1800.f, 602.f, 50.f);
    add(2, 1850.f, 572.f, 40.f);
    add(2, 0.f, 945.f, 540.f);     // ground, left
    add(2, 680.f, 935.f, 650.f);   // ground, middle
    add(2, 1330.f, 960.f, 230.f);
    add(2, 1560.f, 990.f, 380.f);  // ground, right
    add(1, 1860.f, 940.f, 80.f);   // little bridge between scene6 and scene7

    // Safety floor so nobody falls forever
    addSolid(0.f, 1070.f, RUINS_WIDTH, 16.f);
}

void Level::buildCathedral()
{
    for (int i = 0; i < 3; i++)
    {
        float x = i * SCENE_WIDTH;
        addSolid(x, 900.f, SCENE_WIDTH, 40.f);
        addSolid(x + 160.f, 750.f, 250.f, 16.f);
        addSolid(x + 520.f, 630.f, 230.f, 16.f);
        addSolid(x + 900.f, 700.f, 250.f, 16.f);
    }
}

bool Level::isCathedral() const { return region == Region::Cathedral; }
void Level::enterCathedral() { setRegion(Region::Cathedral); }
Region Level::getRegion() const { return region; }

sf::Vector2f Level::getSpawn() const
{
    if (region == Region::RedMoonRuins)
        return { 120.f, 790.f };   // on the left floor of scene5
    return { 100.f, 200.f };
}

bool Level::hasDoor() const { return region != Region::Cathedral; }

Region Level::getNextRegion() const
{
    if (region == Region::AshenBarrow) return Region::RedMoonRuins;
    return Region::Cathedral;
}

// The "Press E" area around the exit gate of each region
sf::FloatRect Level::getDoorTrigger() const
{
    if (region == Region::AshenBarrow)
        return sf::FloatRect({ 6184.f, -90.f }, { 350.f, 1000.f });
    if (region == Region::RedMoonRuins)   // the glowing portal at the end of scene7
        return sf::FloatRect({ 2.f * RUINS_SCENE_WIDTH + 1600.f, 330.f }, { 300.f, 280.f });
    return sf::FloatRect({ 0.f, 0.f }, { 0.f, 0.f });
}


void Level::draw(sf::RenderWindow& window)
{
    if (region == Region::AshenBarrow)
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
    else if (region == Region::RedMoonRuins)
    {
        for (int i = 0; i < 3; i++)
        {
            sf::Sprite s(pictures[i + 10]);
            s.setPosition({i * RUINS_SCENE_WIDTH, 0.f});
            s.setScale({RUINS_SCENE_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
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
{
    if (region == Region::Cathedral)    return 3.f * SCENE_WIDTH;
    if (region == Region::RedMoonRuins) return RUINS_WIDTH;
    return EXTERIOR_WIDTH;
}
float Level::getTop() const 
{ return 0.f; }
float Level::getHeight() const 
{ return HEIGHT; }
