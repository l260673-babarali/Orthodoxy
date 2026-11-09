#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../../Game/Player/Player.h"
#include "../../Game/Levels/Level.h"
#include "../../Game/Enemies/Enemy.h"
#include "../../Game/Combat/Projectile.h"
#include "../../Game/Combat/Spike.h"
#include "../Graphics/Camera.h"

class Engine
{
private:
    sf::RenderWindow window;
    Player player;
    Level level;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Spike> spikes;
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