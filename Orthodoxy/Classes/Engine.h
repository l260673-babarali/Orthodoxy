#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Player.h"
#include "Level.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Spike.h"
#include "Camera.h"
#include <stdexcept>
#include <iostream>

class Engine
{
private:
    sf::RenderWindow window;
    Player player;
    Level level;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Spike> spikes;
    sf::Texture healthFullTexture;
    sf::Texture healthEmptyTexture;
    sf::Texture entranceBackgroundTexture;
    std::unique_ptr<sf::Sprite> entranceBackground;
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