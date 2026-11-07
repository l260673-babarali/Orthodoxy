#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../../Game/Player/Player.h"
#include "../../Game/Levels/Level.h"
#include "../../Game/Enemies/Enemy.h"
#include "../Graphics/Camera.h"

class Engine
{
private:
    sf::RenderWindow window;
    Player player;
    Level level;
    std::vector<Enemy> enemies;
    sf::Clock clock;
    Camera camera;

    void processEvents();
    void update();
    void render();
    void drawHUD();

public:
    Engine();
    void run();
};