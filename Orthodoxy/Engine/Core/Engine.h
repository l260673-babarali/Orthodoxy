#pragma once
#include <SFML/Graphics.hpp>
#include "../../Game/Player/Player.h"
#include "../../Game/Levels/Level.h"
#include "../Graphics/Camera.h"

class Engine
{
private:
    sf::RenderWindow window;
    Player player;
    Level level;
    sf::Clock clock;
    Camera camera;

    void processEvents();
    void update();
    void render();

public:
    Engine();
    void run();
};