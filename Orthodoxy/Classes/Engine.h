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

    void processEvents();
    void update();
    void render();
    void drawHUD();

    void startGame();
    void handleKey(sf::Keyboard::Key key);
    void activateMenuItem();
    std::vector<std::string> menuItems() const;
    void drawMenu(const std::string& title);

public:
    Engine();

    void run();
};
