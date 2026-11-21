#include "../Classes/Level.h"
#include <iostream>
#include <string>
#include <algorithm>

const float OLD_SCALE = 1.5f;
// Ek purane scene ki world width = image width (543) x scaling factor.
const float OLD_SCENE = 543.f * OLD_SCALE;
const float SCENE_WIDTH = 1086.f; // each Cathedral picture (724 x 724) is drawn 1.5x bigger = 1086 x 1086
const float CONTINUATION_WIDTH = 1086.f;
const float HEIGHT = 1086.f;
const float EXTERIOR_WIDTH = 2172.f * OLD_SCALE + 3.f * CONTINUATION_WIDTH;
const float RUINS_SCENE_WIDTH = 1940.f; // each red-moon-ruins picture (scene5, 6, 7) is stretched to this width
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

// A smooth ramp (a staircase you can simply walk on). It is built from thin 4 px wide columns whose
// tops follow the line from (x1, y1) to (x2, y2). The player and the enemies climb these small
// steps by themselves (see max_step_up in Player.h), so it feels like walking up a slope.
void Level::addRamp(float x1, float y1, float x2, float y2)
{
    if (x2 < x1)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    const float column = 4.f;
    for (float x = x1; x < x2; x += column)
    {
        float width = std::min(column, x2 - x) + 1.f;
        float t = (x + width * 0.5f - x1) / (x2 - x1);     // 0 at the start, 1 at the end of the ramp
        addSolid(x, y1 + (y2 - y1) * t, width, 12.f);
    }
}

Level::Level()
{
    pictures.resize(14);
    // The first zone is made of 3 pictures side by side (each one is drawn 1086 x 1086)
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/zone1_part"
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
        // The Cathedral is made of 3 pictures side by side
        std::string name = "Assets/Sprites/Backgrounds/Cathedral/cathedral_part" + std::to_string(i + 1) + ".png";
        if (!pictures[i + 7].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    for (int i = 0; i < 3; i++)
    {
        std::string name = "Assets/Sprites/Backgrounds/Ashen Barrow/scene" + std::to_string(i + 5) + ".png";
        if (!pictures[i + 10].loadFromFile(name))
            std::cout << "Missing: " << name << std::endl;
    }
    // The boss room picture (the Draconic Lord's throne hall)
    if (!pictures[13].loadFromFile("Assets/Sprites/Backgrounds/Boss Room/boss_room.jpg"))
        std::cout << "Missing: boss_room.jpg" << std::endl;
    for (int i = 0; i < 14; i++) pictures[i].setSmooth(false);

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
    else if (region == Region::Cathedral)    buildCathedral();
    else if (region == Region::Cemetery)     buildCemetery();
    else                                     buildBossRoom();
}

// Ashen platforms ke addSolid(x,y,width,height) world coordinates hain; OLD_SCALE x position, y aur width ko same proportion se barhata hai. Values ko change nahi kiya.
void Level::buildAshenBarrow()
{
    // First zone (the 3 new pictures, world coordinates measured from the pictures)
    addSolid(25.f, 252.f, 327.f, 12.f);
    addSolid(330.f, 387.f, 312.f, 12.f);
    addSolid(757.f, 433.f, 409.f, 12.f);
    addSolid(100.f, 580.f, 435.f, 12.f);
    addSolid(640.f, 600.f, 250.f, 12.f);
    addSolid(110.f, 818.f, 430.f, 12.f);
    addSolid(525.f, 925.f, 400.f, 12.f);
    addSolid(855.f, 850.f, 436.f, 12.f);
    addSolid(1086.f, 312.f, 373.f, 12.f);
    addSolid(1151.f, 547.f, 240.f, 12.f);
    addSolid(1216.f, 700.f, 350.f, 12.f);
    addSolid(1550.f, 624.f, 110.f, 12.f);
    addSolid(1609.f, 585.f, 277.f, 12.f);
    addSolid(1876.f, 535.f, 190.f, 12.f);
    addSolid(2016.f, 474.f, 236.f, 12.f);
    addSolid(1576.f, 885.f, 475.f, 12.f);
    addSolid(1745.f, 863.f, 100.f, 12.f);
    addSolid(1996.f, 745.f, 176.f, 12.f);
    addSolid(2187.f, 348.f, 375.f, 12.f);
    addSolid(2447.f, 563.f, 325.f, 12.f);
    addSolid(2172.f, 683.f, 410.f, 12.f);
    addSolid(2490.f, 815.f, 317.f, 12.f);
    addSolid(2815.f, 855.f, 147.f, 12.f);
    addSolid(2932.f, 540.f, 326.f, 12.f);
    addSolid(2757.f, 670.f, 230.f, 12.f);
    // Smooth stairs (world coordinates): each ramp joins the two platforms it connects
    addRamp(538.f, 822.f, 615.f, 925.f);
    addRamp(525.f, 688.f, 640.f, 603.f);
    addRamp(770.f, 925.f, 852.f, 850.f);
    addRamp(925.f, 425.f, 1088.f, 312.f);
    addRamp(1421.f, 695.f, 1552.f, 624.f);
    addRamp(1836.f, 588.f, 1886.f, 538.f);
    addRamp(1966.f, 535.f, 2022.f, 476.f);
    addRamp(1716.f, 885.f, 1745.f, 863.f);
    addRamp(1960.f, 882.f, 2010.f, 832.f);
    addRamp(2110.f, 474.f, 2192.f, 352.f);
    addRamp(2120.f, 745.f, 2185.f, 684.f);
    addRamp(2802.f, 818.f, 2830.f, 853.f);
}

// Region 5: the cemetery-and-waterfall part of Ashen Barrow. Three pictures of 1086 x 1086
// (continuation1-3); the gate to Red Moon Ruins is at the far right.
void Level::buildCemetery()
{
    // Distinct cemetery-and-waterfall continuation.d
    // Each raw pixel is multiplied by 1.5 for the game world.
    float start = 0.f;

  
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

    // Smooth stairs (world coordinates)
    addRamp(752.f, 902.f, 937.f, 735.f);
    addRamp(1142.f, 727.f, 1312.f, 554.f);
    addRamp(2172.f, 822.f, 2322.f, 688.f);
    addRamp(2812.f, 512.f, 2982.f, 326.f);
}

// ---------------------------------------------------------------------------
// Region 2: Red Moon Ruins. Three pictures side by side (scene5, scene6, scene7),
// each stretched to 1940 x 1086. All numbers below are positions INSIDE one picture;
// "scene" is 0, 1 or 2 and moves the platform to the right picture.
// The numbers were estimated from the pictures: switch showPlatforms on in
// Level::draw (it is on by default) and click in the game to print the real
// coordinates, then nudge any platform that is a few pixels off.
// ---------------------------------------------------------------------------
// Red Moon addSolid mein local scene X + sceneIndex x RUINS_SCENE_WIDTH = world X. Platform values ko change nahi kiya.
void Level::buildRedMoonRuins()
{
    // ----- scene5 (first picture) -----
    addSolid(0.f, 878.f, 745.f, 14.f);     // left floor
    addSolid(0.f, 705.f, 210.f, 14.f);     // ledge above the stairs
    addRamp(205.f, 722.f, 372.f, 876.f);   // stairs going down
    addSolid(245.f, 595.f, 280.f, 14.f);   // torch ledge
    addSolid(215.f, 484.f, 110.f, 14.f);   // wooden swing
    addSolid(0.f, 322.f, 385.f, 14.f);     // top-left ledge + plank
    addSolid(510.f, 618.f, 330.f, 14.f);   // rope bridge
    addSolid(645.f, 415.f, 85.f, 14.f);    // hanging swing
    addSolid(715.f, 512.f, 415.f, 14.f);   // big arch top
    addRamp(1128.f, 538.f, 1262.f, 624.f); // stairs going down to the right
    addSolid(1100.f, 625.f, 220.f, 14.f);
    addSolid(1290.f, 575.f, 650.f, 14.f);  // long right platform (continues into scene6)
    addSolid(1340.f, 378.f, 85.f, 14.f);   // broken pillar top
    addSolid(1465.f, 275.f, 435.f, 14.f);  // high right platform
    addSolid(1500.f, 772.f, 60.f, 14.f);   // short pillar
    addSolid(610.f, 995.f, 790.f, 14.f);   // bottom middle ground
    addSolid(1400.f, 905.f, 540.f, 14.f);  // bottom right ground

    // ----- scene6 (second picture) -----
    addSolid(1.f * RUINS_SCENE_WIDTH + 0.f, 495.f, 440.f, 14.f);     // portal ledge
    addRamp(1.f * RUINS_SCENE_WIDTH + 438.f, 524.f, 1.f * RUINS_SCENE_WIDTH + 506.f, 606.f);   // stairs going down
    addSolid(1.f * RUINS_SCENE_WIDTH + 500.f, 608.f, 190.f, 14.f);
    addRamp(1.f * RUINS_SCENE_WIDTH + 688.f, 612.f, 1.f * RUINS_SCENE_WIDTH + 735.f, 703.f);
    addSolid(1.f * RUINS_SCENE_WIDTH + 705.f, 705.f, 275.f, 14.f);   // torch table
    addSolid(1.f * RUINS_SCENE_WIDTH + 585.f, 548.f, 225.f, 14.f);   // small upper platform
    addSolid(1.f * RUINS_SCENE_WIDTH + 940.f, 575.f, 180.f, 14.f);
    addSolid(1.f * RUINS_SCENE_WIDTH + 1120.f, 565.f, 295.f, 14.f);  // brazier platform
    addSolid(1.f * RUINS_SCENE_WIDTH + 1260.f, 775.f, 240.f, 14.f);
    addSolid(1.f * RUINS_SCENE_WIDTH + 1480.f, 632.f, 460.f, 14.f);  // right platform with the dead tree
    addSolid(1.f * RUINS_SCENE_WIDTH + 0.f, 748.f, 230.f, 14.f);     // lower-left ledge
    addRamp(1.f * RUINS_SCENE_WIDTH + 226.f, 760.f, 1.f * RUINS_SCENE_WIDTH + 320.f, 876.f);   // stairs going down
    addSolid(1.f * RUINS_SCENE_WIDTH + 300.f, 878.f, 250.f, 14.f);
    addSolid(1.f * RUINS_SCENE_WIDTH + 550.f, 905.f, 50.f, 14.f);
    addSolid(1.f * RUINS_SCENE_WIDTH + 0.f, 928.f, 300.f, 14.f);     // ground, left
    addSolid(1.f * RUINS_SCENE_WIDTH + 600.f, 938.f, 590.f, 14.f);   // ground, middle
    addSolid(1.f * RUINS_SCENE_WIDTH + 1190.f, 972.f, 310.f, 14.f);
    addSolid(1.f * RUINS_SCENE_WIDTH + 1500.f, 925.f, 440.f, 14.f);  // ground, right

    // ----- scene7 (third picture, the exit portal is on the right) -----
    addSolid(2.f * RUINS_SCENE_WIDTH + 0.f, 688.f, 460.f, 14.f);     // left stone ledge
    addSolid(2.f * RUINS_SCENE_WIDTH + 460.f, 678.f, 140.f, 14.f);   // wooden bridge
    addRamp(2.f * RUINS_SCENE_WIDTH + 598.f, 676.f, 2.f * RUINS_SCENE_WIDTH + 792.f, 554.f);   // stairs going up
    addSolid(2.f * RUINS_SCENE_WIDTH + 790.f, 552.f, 330.f, 14.f);   // banner platform
    addSolid(2.f * RUINS_SCENE_WIDTH + 1120.f, 555.f, 170.f, 14.f);  // bridge
    addSolid(2.f * RUINS_SCENE_WIDTH + 1290.f, 585.f, 240.f, 14.f);
    addRamp(2.f * RUINS_SCENE_WIDTH + 1524.f, 585.f, 2.f * RUINS_SCENE_WIDTH + 1566.f, 548.f);
    addSolid(2.f * RUINS_SCENE_WIDTH + 1560.f, 545.f, 380.f, 14.f);  // portal ledge
    addSolid(2.f * RUINS_SCENE_WIDTH + 1410.f, 735.f, 230.f, 14.f);  // lower right ledge
    addRamp(2.f * RUINS_SCENE_WIDTH + 1596.f, 735.f, 2.f * RUINS_SCENE_WIDTH + 1880.f, 550.f);  // big stairs going up to the portal
    addSolid(2.f * RUINS_SCENE_WIDTH + 0.f, 945.f, 540.f, 14.f);     // ground, left
    addSolid(2.f * RUINS_SCENE_WIDTH + 680.f, 935.f, 650.f, 14.f);   // ground, middle
    addSolid(2.f * RUINS_SCENE_WIDTH + 1330.f, 960.f, 230.f, 14.f);
    addSolid(2.f * RUINS_SCENE_WIDTH + 1560.f, 990.f, 380.f, 14.f);  // ground, right
    addSolid(1.f * RUINS_SCENE_WIDTH + 1860.f, 940.f, 80.f, 14.f);   // little bridge between scene6 and scene7

    // Safety floor so nobody falls forever
    addSolid(0.f, 1070.f, RUINS_WIDTH, 16.f);
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
    const float k = SCENE_WIDTH / 724.f;
    // A flat platform: top-left corner (x, y) inside picture 'part', and its width
    auto ledge = [&](int part, float x, float y, float width)
    {
        addSolid(part * SCENE_WIDTH + x * k, y * k, width * k, 12.f);
    };
    // A smooth ramp from (x1, y1) to (x2, y2) inside picture 'part' (original picture pixels)
    auto ramp = [&](int part, float x1, float y1, float x2, float y2)
    {
        addRamp(part * SCENE_WIDTH + x1 * k, y1 * k, part * SCENE_WIDTH + x2 * k, y2 * k);
    };

    // ----- Part 1 -----
    ledge(0, 0.f, 245.f, 175.f);      // top-left balcony
    ledge(0, 565.f, 258.f, 159.f);    // top-right ledge (torch bowl)
    ledge(0, 208.f, 365.f, 377.f);    // big middle balcony
    ledge(0, 150.f, 455.f, 163.f);    // left middle ledge
    ledge(0, 0.f, 523.f, 170.f);      // left low platform (brazier)
    ledge(0, 235.f, 592.f, 365.f);    // main floor
    ledge(0, 605.f, 478.f, 119.f);    // right middle ledge
    ledge(0, 640.f, 645.f, 84.f);     // rubble ledge at the bottom right
    ramp(0, 85.f, 505.f, 157.f, 453.f);    // up to the left middle ledge
    ramp(0, 172.f, 526.f, 235.f, 590.f);    // down to the main floor
    ramp(0, 468.f, 590.f, 606.f, 480.f);    // from the floor up to the right middle ledge
    ramp(0, 470.f, 363.f, 566.f, 262.f);    // from the big balcony up to the top-right ledge

    // ----- Part 2 -----
    ledge(1, 0.f, 258.f, 90.f);       // statue ledge (continues from the top-right ledge)
    ledge(1, 190.f, 388.f, 385.f);    // big middle balcony with the brazier
    ledge(1, 0.f, 476.f, 178.f);      // left middle ledge
    ledge(1, 425.f, 365.f, 92.f);     // little balcony before the stairs
    ledge(1, 540.f, 318.f, 184.f);    // upper right ledge
    ledge(1, 540.f, 548.f, 184.f);    // right plaza floor
    ledge(1, 262.f, 585.f, 283.f);    // middle floor
    ledge(1, 0.f, 643.f, 285.f);      // bottom left floor
    ramp(1, 95.f, 476.f, 197.f, 390.f);    // up to the big balcony
    ramp(1, 490.f, 363.f, 540.f, 320.f);    // up to the upper right ledge
    ramp(1, 180.f, 641.f, 262.f, 585.f);    // rubble steps up to the middle floor

    // ----- Part 3 (the purple portal to the boss is on the right) -----
    ledge(2, 18.f, 258.f, 262.f);     // gallery with the candles
    ledge(2, 0.f, 320.f, 46.f);       // small ledge on the left
    ledge(2, 0.f, 548.f, 605.f);      // big plaza floor
    ledge(2, 608.f, 525.f, 116.f);    // raised block on the right
    ramp(2, 505.f, 546.f, 614.f, 400.f);    // steps up to the portal
    ledge(2, 614.f, 398.f, 86.f);     // the landing in front of the portal

    // Safety floor so nobody falls forever
    addSolid(0.f, 1070.f, 3.f * SCENE_WIDTH, 16.f);
}

// The boss room is one flat arena (its picture is boss_room.jpg, stretched to the arena size).
void Level::buildBossRoom()
{
    addSolid(0.f, 900.f, BOSS_ROOM_WIDTH, 40.f);
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
        return sf::FloatRect({ 2926.f, -90.f }, { 350.f, 1000.f });
    if (region == Region::RedMoonRuins)   // the glowing portal at the end of scene7
        return sf::FloatRect({ 2.f * RUINS_SCENE_WIDTH + 1600.f, 330.f }, { 300.f, 280.f });
    if (region == Region::Cathedral)      // the purple portal at the end of Part 3 (leads to the boss)
        return sf::FloatRect({ 3050.f, 330.f }, { 208.f, 270.f });
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
    bool showPlatforms = false;
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
