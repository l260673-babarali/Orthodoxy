#include "../Classes/Level.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>

const float OLD_SCALE = 1.5f;
// Ek purane scene ki world width = image width (543) x scaling factor.
const float OLD_SCENE = 543.f * OLD_SCALE;
const float SCENE_WIDTH = 1086.f; // each Cathedral picture (724 x 724) is drawn 1.5x bigger = 1086 x 1086
const float CONTINUATION_WIDTH = 1086.f;
const float HEIGHT = 1086.f;
const float EXTERIOR_WIDTH = 2172.f * OLD_SCALE + 3.f * CONTINUATION_WIDTH;
const float RUINS_SCENE_WIDTH = 1237.f; // each Red Moon picture (1365 x 1198) is drawn 1237 x 1086, so it is not stretched
// Red Moon total width = 3 scene widths; 3 backgrounds side by side hain.
const float RUINS_WIDTH = 3.f * RUINS_SCENE_WIDTH;
const float BOSS_ROOM_WIDTH = 1920.f; // must match arena_left / arena_right of the DraconicLord in Enemy.h

void Level::addSolid(float x, float y, float width, float height)
{
    sf::RectangleShape box({width, height});
    box.setPosition({x, y});
    box.setFillColor(sf::Color::Transparent);
    box.setOutlineColor(sf::Color::Green);
    box.setOutlineThickness(2.f);
    platforms.push_back(box);
}

// A smooth ramp (a staircase you can simply walk on). addRamp only remembers the line; buildRamps()
// (called at the end of setRegion) then makes each end of the ramp meet the top of the platform it
// joins, so the knight never has to climb a step that is too high or bump his head under a ledge.
// The ramp itself is made of thin 4 px wide columns whose tops follow the line. The player and the
// enemies climb these small steps by themselves (see max_step_up in Player.h).
void Level::addRamp(float x1, float y1, float x2, float y2)
{
    ramps.push_back({ x1, y1, x2, y2 });
}

void Level::buildRamps()
{
    const size_t real_platforms = platforms.size();   // only the real platforms count when snapping
    for (RampLine& r : ramps)
    {
        if (r.x2 < r.x1)
        {
            std::swap(r.x1, r.x2);
            std::swap(r.y1, r.y2);
        }
        for (int end = 0; end < 2; end++)
        {
            float& x = (end == 0) ? r.x1 : r.x2;
            float& y = (end == 0) ? r.y1 : r.y2;
            float best_distance = 41.f;   // only platforms within 40 px of the end count
            float best_top = y;
            for (size_t i = 0; i < real_platforms; i++)
            {
                sf::FloatRect box = platforms[i].getGlobalBounds();
                if (x < box.position.x - 10.f || x > box.position.x + box.size.x + 10.f)
                    continue;
                float distance = std::abs(box.position.y - y);
                if (distance < best_distance)
                {
                    best_distance = distance;
                    best_top = box.position.y;
                }
            }
            y = best_top;
        }

        const float column = 4.f;
        for (float x = r.x1; x < r.x2; x += column)
        {
            float width = std::min(column, r.x2 - x) + 1.f;
            float t = (x + width * 0.5f - r.x1) / (r.x2 - r.x1);   // 0 at the start, 1 at the end of the ramp
            addSolid(x, r.y1 + (r.y2 - r.y1) * t, width, 12.f);
        }
    }
    ramps.clear();
}

Level::Level()
{
    pictures.resize(14);
    // The first zone is made of 3 pictures side by side (each one is drawn 1086 x 1086)
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/zone1_part"
                         + std::to_string(i + 1) + ".jpg";
        if (!pictures[i].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/zone2_part" + std::to_string(i + 1) + ".jpg";
        if (!pictures[i + 4].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 3; i++)
    {
        // The Cathedral is made of 3 pictures side by side
        std::string name = "Assets/Sprites/Backgrounds/Cathedral/cathedral_part" + std::to_string(i + 1) + ".jpg";
        if (!pictures[i + 7].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/zone3_part" + std::to_string(i + 1) + ".jpg";
        if (!pictures[i + 10].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    // The boss room picture (the Draconic Lord's throne hall)
    if (!pictures[13].loadFromFile("Assets/Sprites/Backgrounds/Boss Room/boss_room.jpg"))
        std::cout << "Missing: boss_room.jpg" << std::endl;
    for (int i = 0; i < 14; i++) pictures[i].setSmooth(true);

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
    ramps.clear();
    if (region == Region::AshenBarrow)       buildAshenBarrow();
    else if (region == Region::RedMoonRuins) buildRedMoonRuins();
    else if (region == Region::Cathedral)    buildCathedral();
    else if (region == Region::Cemetery)     buildCemetery();
    else                                     buildBossRoom();
    buildRamps();
}

// Ashen platforms ke addSolid(x,y,width,height) world coordinates hain; OLD_SCALE x position, y aur width ko same proportion se barhata hai. Values ko change nahi kiya.
void Level::buildAshenBarrow()
{
    // Zone 1 - Ashen Barrow (3 pictures, 1086 wide each). Positions measured from the pictures (top edge of each stone rim).
    addSolid(0.f, 250.f, 352.f, 12.f);
    addSolid(330.f, 387.f, 312.f, 12.f);
    addSolid(645.f, 334.f, 155.f, 12.f);
    addSolid(755.f, 433.f, 410.f, 12.f);
    addSolid(100.f, 578.f, 437.f, 12.f);
    addSolid(640.f, 602.f, 250.f, 12.f);
    addSolid(110.f, 817.f, 430.f, 12.f);
    addSolid(525.f, 921.f, 400.f, 12.f);
    addSolid(900.f, 850.f, 386.f, 12.f);
    addSolid(0.f, 803.f, 115.f, 12.f);
    addSolid(1086.f, 306.f, 372.f, 12.f);
    addSolid(1151.f, 546.f, 240.f, 12.f);
    addSolid(1216.f, 698.f, 350.f, 12.f);
    addSolid(1486.f, 624.f, 183.f, 12.f);
    addSolid(1606.f, 578.f, 280.f, 12.f);
    addSolid(1876.f, 528.f, 190.f, 12.f);
    addSolid(1976.f, 470.f, 278.f, 12.f);
    addSolid(1996.f, 744.f, 258.f, 12.f);
    addSolid(1606.f, 882.f, 445.f, 12.f);
    addSolid(2187.f, 350.f, 377.f, 12.f);
    addSolid(2444.f, 561.f, 328.f, 12.f);
    addSolid(2172.f, 680.f, 408.f, 12.f);
    addSolid(2490.f, 814.f, 317.f, 12.f);
    addSolid(2800.f, 856.f, 172.f, 12.f);
    addSolid(2757.f, 669.f, 215.f, 12.f);
    addSolid(2932.f, 543.f, 326.f, 12.f);
    addRamp(925.f, 428.f, 1086.f, 308.f);
    addRamp(522.f, 688.f, 640.f, 606.f);
    addRamp(525.f, 822.f, 610.f, 898.f);
    addRamp(772.f, 893.f, 900.f, 850.f);
    addRamp(1421.f, 692.f, 1500.f, 626.f);
    addRamp(1841.f, 572.f, 1900.f, 533.f);
    addRamp(1966.f, 528.f, 2021.f, 472.f);
    addRamp(2105.f, 468.f, 2195.f, 350.f);
    addRamp(2800.f, 820.f, 2835.f, 855.f);

    // Safety floor so nobody falls forever
    addSolid(0.f, 1070.f, 3258.f, 16.f);
}

// Region 5: the cemetery-and-waterfall part of Ashen Barrow. Three pictures of 1086 x 1086
// (continuation1-3); the gate to Red Moon Ruins is at the far right.
void Level::buildCemetery()
{
    // Zone 2 - Ashen Cemetery (3 pictures, 1086 wide each). Positions measured from the pictures (top edge of each stone rim).
    addSolid(0.f, 359.f, 410.f, 12.f);
    addSolid(310.f, 552.f, 620.f, 12.f);
    addSolid(0.f, 812.f, 200.f, 12.f);
    addSolid(200.f, 836.f, 325.f, 12.f);
    addSolid(520.f, 935.f, 505.f, 12.f);
    addSolid(920.f, 751.f, 490.f, 12.f);
    addSolid(1336.f, 571.f, 495.f, 12.f);
    addSolid(1781.f, 445.f, 391.f, 12.f);
    addSolid(1921.f, 871.f, 381.f, 12.f);
    addSolid(1696.f, 997.f, 270.f, 12.f);
    addSolid(2587.f, 543.f, 425.f, 12.f);
    addSolid(2967.f, 325.f, 291.f, 12.f);
    addSolid(2990.f, 733.f, 268.f, 12.f);
    addSolid(2292.f, 708.f, 447.f, 12.f);
    addRamp(745.f, 905.f, 925.f, 778.f);
    addRamp(1150.f, 735.f, 1336.f, 600.f);
    addRamp(2745.f, 522.f, 2975.f, 335.f);
    addRamp(2178.f, 832.f, 2300.f, 712.f);

    // Safety floor so nobody falls forever
    addSolid(0.f, 1070.f, 3258.f, 16.f);
}

// ---------------------------------------------------------------------------
// Region 2: Red Moon Ruins. Three pictures side by side (scene5, scene6, scene7),
// each drawn 1237 x 1086. All numbers below are positions INSIDE one picture;
// "scene" is 0, 1 or 2 and moves the platform to the right picture.
// The numbers were estimated from the pictures: switch showPlatforms on in
// Level::draw (it is on by default) and click in the game to print the real
// coordinates, then nudge any platform that is a few pixels off.
// ---------------------------------------------------------------------------
// Red Moon addSolid mein local scene X + sceneIndex x RUINS_SCENE_WIDTH = world X. Platform values ko change nahi kiya.
void Level::buildRedMoonRuins()
{
    // Zone 3 - Red Moon Ruins (3 pictures, 1237 wide each). Positions measured from the pictures (top edge of each stone rim).
    addSolid(0.f, 296.f, 238.f, 12.f);
    addSolid(350.f, 493.f, 352.f, 12.f);
    addSolid(0.f, 665.f, 392.f, 12.f);
    addSolid(400.f, 814.f, 405.f, 12.f);
    addSolid(800.f, 925.f, 437.f, 12.f);
    addSolid(700.f, 469.f, 637.f, 12.f);
    addSolid(1337.f, 598.f, 200.f, 12.f);
    addSolid(900.f, 708.f, 867.f, 12.f);
    addSolid(1907.f, 827.f, 380.f, 12.f);
    addSolid(2287.f, 761.f, 990.f, 12.f);
    addSolid(2544.f, 456.f, 330.f, 12.f);
    addSolid(3170.f, 595.f, 541.f, 12.f);
    addRamp(392.f, 668.f, 480.f, 792.f);
    addRamp(770.f, 845.f, 865.f, 923.f);
    addRamp(1472.f, 599.f, 1682.f, 697.f);
    addRamp(2814.f, 480.f, 3176.f, 598.f);
    addRamp(2989.f, 738.f, 3194.f, 612.f);

    // Safety floor so nobody falls forever
    addSolid(0.f, 1070.f, 3711.f, 16.f);
}

// ---------------------------------------------------------------------------
// Region 3: Cathedral. Three pictures side by side (Cathedral_Part_1, 2, 3), each 724 x 724
// and drawn 1.5x bigger (1086 x 1086). The numbers below are positions in the ORIGINAL
// 724 x 724 pictures; k turns them into game coordinates and 'part' (0, 1, 2) moves a
// platform to the right picture. They were measured from the pictures: keep the green
// outlines on (showPlatforms in Level::draw) and nudge any platform that looks a bit off.
// ---------------------------------------------------------------------------
void Level::buildCathedral()
{
    // Zone 4 - the Cathedral (3 pictures, 1086 wide each). Positions measured from the pictures (top edge of each stone rim).
    addSolid(0.f, 368.f, 282.f, 12.f);
    addSolid(340.f, 563.f, 620.f, 12.f);
    addSolid(235.f, 682.f, 260.f, 12.f);
    addSolid(0.f, 805.f, 245.f, 12.f);
    addSolid(220.f, 932.f, 800.f, 12.f);
    addSolid(890.f, 389.f, 396.f, 12.f);
    addSolid(955.f, 759.f, 450.f, 12.f);
    addSolid(1416.f, 604.f, 620.f, 12.f);
    addSolid(1966.f, 486.f, 391.f, 12.f);
    addSolid(1086.f, 1040.f, 480.f, 12.f);
    addSolid(1486.f, 939.f, 450.f, 12.f);
    addSolid(2172.f, 835.f, 700.f, 12.f);
    addSolid(2985.f, 613.f, 273.f, 12.f);
    addSolid(2302.f, 393.f, 260.f, 12.f);
    addRamp(105.f, 795.f, 240.f, 688.f);
    addRamp(245.f, 832.f, 392.f, 925.f);
    addRamp(640.f, 562.f, 905.f, 395.f);
    addRamp(745.f, 902.f, 960.f, 775.f);
    addRamp(1216.f, 742.f, 1426.f, 608.f);
    addRamp(1796.f, 590.f, 1966.f, 495.f);
    addRamp(1406.f, 1000.f, 1510.f, 932.f);
    addRamp(2880.f, 806.f, 2985.f, 615.f);
    addRamp(1000.f, 938.f, 1090.f, 1040.f);

    // Safety floor so nobody falls forever
    addSolid(0.f, 1070.f, 3258.f, 16.f);
}

// The boss room is one flat arena (its picture is boss_room.jpg, stretched to the arena size).
void Level::buildBossRoom()
{
    addSolid(0.f, 940.f, BOSS_ROOM_WIDTH, 40.f);   // the stone floor of the new throne-hall picture
}

bool Level::isCathedral() const { return region == Region::Cathedral; }
void Level::enterCathedral() { setRegion(Region::Cathedral); }
Region Level::getRegion() const { return region; }

sf::Vector2f Level::getSpawn() const
{
    if (region == Region::RedMoonRuins)
        return { 120.f, 500.f };   // the knight falls onto the first safe platform
    if (region == Region::BossRoom)
        return { 150.f, 700.f };   // the knight drops onto the arena floor
    return { 100.f, 200.f };
}

bool Level::hasDoor() const { return region != Region::BossRoom; }

Region Level::getNextRegion() const
{
    if (region == Region::AshenBarrow) return Region::Cemetery;
    if (region == Region::Cemetery) return Region::RedMoonRuins;
    if (region == Region::RedMoonRuins) return Region::Cathedral;
    return Region::BossRoom;
}

// The "Press E" area around the exit gate of each region
sf::FloatRect Level::getDoorTrigger() const
{
    if (region == Region::AshenBarrow)    // the glowing gate painted at the end of the third picture
        return sf::FloatRect({ 2990.f, 300.f }, { 240.f, 260.f });
    if (region == Region::Cemetery)       // the gate to Red Moon Ruins at the far right
        return sf::FloatRect({ 3030.f, 140.f }, { 230.f, 200.f });
    if (region == Region::RedMoonRuins)   // the glowing portal at the end of scene7
        return sf::FloatRect({ 3400.f, 340.f }, { 230.f, 280.f });
    if (region == Region::Cathedral)      // the purple portal at the end of Part 3 (leads to the boss)
        return sf::FloatRect({ 3010.f, 330.f }, { 200.f, 290.f });
    return sf::FloatRect({ 0.f, 0.f }, { 0.f, 0.f });
}


void Level::draw(sf::RenderWindow& window)
{
    if (region == Region::AshenBarrow)
    {
        for (int i = 0; i < 3; i++)
        {
            sf::Sprite s(pictures[i]);
            s.setPosition({i * SCENE_WIDTH, 0.f});
            s.setScale({SCENE_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
            window.draw(s);
        }
    }
    else if (region == Region::Cemetery)
    {
        for (int i = 0; i < 3; i++)
        {
            sf::Sprite s(pictures[i + 4]);
            // Continuation background ka world start = pehle four scenes ki total scaled width + continuation index x width.
            s.setPosition({i * CONTINUATION_WIDTH, 0.f});
            // Background scale X = desired world width / original texture width; Y = world height / texture height.
            s.setScale({CONTINUATION_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
            window.draw(s);
        }
    }
    else if (region == Region::RedMoonRuins)
    {
        for (int i = 0; i < 3; i++)
        {
            sf::Sprite s(pictures[i + 10]);
            // Red Moon image ka starting X = image index x ek scene ki world width.
            s.setPosition({i * RUINS_SCENE_WIDTH, 0.f});
            s.setScale({RUINS_SCENE_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
            window.draw(s);
        }
    }
    else if (region == Region::Cathedral)
    {
        for (int i = 0; i < 3; i++)
        {
            sf::Sprite s(pictures[i + 7]);
            s.setPosition({i * SCENE_WIDTH, 0.f});
            s.setScale({SCENE_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
            window.draw(s);
        }
    }
    else
    {
        // Boss room: one picture stretched over the whole arena
        sf::Sprite s(pictures[13]);
        s.setScale({BOSS_ROOM_WIDTH / s.getTexture().getSize().x, HEIGHT / s.getTexture().getSize().y});
        window.draw(s);
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
    if (region == Region::BossRoom)     return BOSS_ROOM_WIDTH;
    if (region == Region::Cathedral)    return 3.f * SCENE_WIDTH;
    if (region == Region::RedMoonRuins) return RUINS_WIDTH;
    return 3.f * SCENE_WIDTH;   // Ashen Barrow and the Cemetery are both 3 pictures wide
}
float Level::getTop() const 
{ return 0.f; }
float Level::getHeight() const 
{ return HEIGHT; }
