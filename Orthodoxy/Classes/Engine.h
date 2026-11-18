#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
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
enum class GameState { MainMenu, Playing, Paused, StageCleared, Upgrades };

// Runs the whole game: window, main loop, menus, and everything that lives in the world
class Engine
{
private:
    sf::RenderWindow window;
    sf::View ui_view;
    Player player;
    Level level;
    std::vector<Enemy*> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Spike> spikes;
    sf::Texture health_full_texture;
    sf::Texture health_empty_texture;
    sf::Texture scroll_texture;
    sf::Clock clock;
    Camera camera;

    // Menu / pause screen
    GameState state = GameState::MainMenu;
    sf::Font font;
    sf::Font title_font;
    bool title_font_loaded = false;
    bool font_loaded = false;
    int menu_index = 0;          // which menu item is highlighted

    int total_kills = 0;
    int stage_kills = 0;
    int stage_relics = 0;
    float stage_seconds = 0.f;
    int combo_hits = 0;
    int best_combo = 0;
    float combo_timer = 0.f;
    static constexpr float combo_window = 2.5f;
    Region cleared_region = Region::AshenBarrow;
    int scrolls = 0;
    int collected_total = 0;
    int upgrade_index = 0;
    bool collected_scrolls[8] = {};
    const sf::Vector2f scroll_positions[8] = {
        { 350.f, 690.f }, { 1650.f, 805.f }, { 4700.f, 495.f },
        { 400.f, 825.f }, { 2700.f, 660.f }, { 4810.f, 505.f },
        { 350.f, 700.f }, { 1700.f, 700.f }
    };


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
    void drawStageCleared();
    void drawScrolls();
    void collectScrolls();
    void drawUpgrades();
    std::string currentObjective() const;
    void recordSwordHit();

    void startGame();
    void clearEnemies();
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
    ~Engine();

    void run();
};
