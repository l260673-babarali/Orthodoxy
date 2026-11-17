#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "Player.h"
#include "Level.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Spike.h"
#include "Camera.h"
#include <stdexcept>
#include <iostream>

// The three screens of the game
enum class GameState { MainMenu, Playing, Paused };

// Runs the whole game: window, main loop, menus, and everything that lives in the world
class Engine
{
private:
    sf::RenderWindow window;
    Player player;
    Level level;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Spike> spikes;
    sf::Texture health_full_texture;
    sf::Texture health_empty_texture;
    sf::Clock clock;
    Camera camera;

    // Menu / pause screen
    GameState state = GameState::MainMenu;
    sf::Font font;
    bool font_loaded = false;
    int menu_index = 0;          // which menu item is highlighted

    // Blink (eyelid) transition used when the player goes through a portal
    bool blinking = false;
    bool blinkSwitched = false;          // true once the new region has been loaded
    float blinkTimer = 0.f;
    Region blinkTarget = Region::AshenBarrow;
    static constexpr float blinkCloseTime = 0.30f;   // eyelids close
    static constexpr float blinkHoldTime = 0.20f;    // fully closed (the region changes here)
    static constexpr float blinkOpenTime = 0.30f;    // eyelids open again

    void processEvents();
    void update();
    void render();
    void drawHUD();

    void startGame();
    void spawnEnemies();                 // enemies for the current region
    void enterRegion(Region region);     // switch region: new level, enemies, player spawn
    void startBlink(Region target);
    void updateBlink(float deltaTime);
    void drawBlink();
    void handleKey(sf::Keyboard::Key key);
    void activateMenuItem();
    std::vector<std::string> menuItems() const;
    void drawMenu(const std::string& title);

public:
    Engine();

    void run();
};
