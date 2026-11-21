#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// The game has 4 regions, played in this order:
//   0 = Ashen Barrow   (the first outdoor level)
//   1 = Red Moon Ruins (scene5, scene6, scene7)
//   2 = Cathedral
//   3 = Boss room (the Draconic Lord's arena)
enum class Region { AshenBarrow = 0, RedMoonRuins = 1, Cathedral = 2, BossRoom = 3 };

class Level {
    // 0-3 scene1-4, 4-6 continuation1-3, 7-9 cathedral1-3, 10-12 scene5-7
    std::vector<sf::Texture> pictures;
    std::vector<sf::RectangleShape> platforms;
    Region region = Region::AshenBarrow;
    void addSolid(float x, float y, float width, float height);
    void buildAshenBarrow();
    void buildRedMoonRuins();
    void buildCathedral();
    void buildBossRoom();
public:
    Level();
    void reset();                       // back to the first region
    void setRegion(Region newRegion);   // switch region (rebuilds the platforms)
    Region getRegion() const;
    void draw(sf::RenderWindow& window);
    const std::vector<sf::RectangleShape>& getPlatforms() const;
    float getWidth() const;
    float getTop() const;
    float getHeight() const;
    bool isCathedral() const;
    void enterCathedral();

    // Where the player starts in the current region (top-left of the player box)
    sf::Vector2f getSpawn() const;
    // The area where "Press E" works. Empty (size 0) if this region has no exit door.
    sf::FloatRect getDoorTrigger() const;
    bool hasDoor() const;
    // Which region the door leads to
    Region getNextRegion() const;
};
