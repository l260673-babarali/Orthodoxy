#include "../Classes/Engine.h"
#include "../Classes/Golem.h"
#include "../Classes/BurstSpirit.h"
#include "../Classes/Phoenix.h"

Engine::Engine()
    : window(sf::VideoMode({ 800u, 600u }), "Orthodoxy")
{
    // Heart pictures for the health bar
    if (!health_full_texture.loadFromFile("Assets/Sprites/Health/Full.png"))
        std::cout << "Could not load Full.png\n";
    if (!health_empty_texture.loadFromFile("Assets/Sprites/Health/Empty.png"))
        std::cout << "Could not load Empty.png\n";

    // Font for the menus
    font_loaded = font.openFromFile("Assets/Fonts/font.ttf")
        || font.openFromFile("C:/Windows/Fonts/arial.ttf");
    if (!font_loaded)
        std::cout << "FONT FAILED TO LOAD!" << std::endl;
}

// Starts (or restarts) a fresh game
void Engine::startGame()
{
    enemies.clear();
    projectiles.clear();
    spikes.clear();

    // Enemies start above the floor and fall down onto it
    enemies.push_back(std::make_unique<Golem>(1000.f, 430.f));
    // Spirits float in the air
    enemies.push_back(std::make_unique<BurstSpirit>(1450.f, 380.f));
    enemies.push_back(std::make_unique<BurstSpirit>(1650.f, 360.f));
    // Phoenix flies high
    enemies.push_back(std::make_unique<Phoenix>(1950.f, 250.f));

    player.reset();
    state = GameState::Playing;
}

// The list of buttons for the current screen
std::vector<std::string> Engine::menuItems() const
{
    if (state == GameState::MainMenu)
        return { "Play", "Quit" };
    return { "Resume", "Main Menu", "Quit" };   // Paused
}

// What happens when Enter is pressed on the highlighted button
void Engine::activateMenuItem()
{
    if (state == GameState::MainMenu)
    {
        if (menu_index == 0) startGame();
        else window.close();
    }
    else if (state == GameState::Paused)
    {
        if (menu_index == 0)
        {
            state = GameState::Playing;     // Resume
        }
        else if (menu_index == 1)
        {
            state = GameState::MainMenu;    // Back to the main menu
            menu_index = 0;
        }
        else
        {
            window.close();                 // Quit
        }
    }
}

// Keyboard presses for pausing and moving through the menus
void Engine::handleKey(sf::Keyboard::Key key)
{
    using K = sf::Keyboard::Key;

    // While playing, the only key we care about is Escape (pause)
    if (state == GameState::Playing)
    {
        if (key == K::Escape)
        {
            state = GameState::Paused;
            menu_index = 0;
        }
        return;
    }

    // In a menu: W/S or Up/Down move the highlight, wrapping around at the ends
    int count = static_cast<int>(menuItems().size());
    if (key == K::W || key == K::Up)   menu_index = (menu_index + count - 1) % count;
    if (key == K::S || key == K::Down) menu_index = (menu_index + 1) % count;
    if (key == K::Enter || key == K::Space) activateMenuItem();
    if (key == K::Escape && state == GameState::Paused)
        state = GameState::Playing;
}

// Draws the title and the buttons (used by both the main menu and the pause screen)
void Engine::drawMenu(const std::string& title)
{
    window.setView(window.getDefaultView());

    // Dim the frozen game behind the pause menu
    if (state == GameState::Paused)
    {
        sf::RectangleShape dim({ 800.f, 600.f });
        dim.setFillColor(sf::Color(0, 0, 0, 160));
        window.draw(dim);
    }

    if (!font_loaded)
        return;

    // Title, centered horizontally at x = 400
    sf::Text title_text(font, title, 56);
    title_text.setFillColor(sf::Color(220, 190, 110));
    sf::FloatRect title_bounds = title_text.getLocalBounds();
    title_text.setOrigin({ title_bounds.position.x + title_bounds.size.x / 2.f, 0.f });
    title_text.setPosition({ 400.f, 120.f });
    window.draw(title_text);

    // Buttons, one under the other. The highlighted one is yellow with > < around it.
    std::vector<std::string> items = menuItems();
    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        bool selected = (i == menu_index);
        sf::Text item_text(font, selected ? "> " + items[i] + " <" : items[i], 32);
        item_text.setFillColor(selected ? sf::Color::Yellow : sf::Color::White);
        sf::FloatRect item_bounds = item_text.getLocalBounds();
        item_text.setOrigin({ item_bounds.position.x + item_bounds.size.x / 2.f, 0.f });
        item_text.setPosition({ 400.f, 280.f + i * 60.f });
        window.draw(item_text);
    }
}

// The main loop: input -> update the world -> draw, over and over
void Engine::run()
{
    while (window.isOpen())
    {
        processEvents();
        update();
        render();
    }
}

void Engine::processEvents()
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            handleKey(key->code);

        // Auto-pause when the window loses focus
        if (event->is<sf::Event::FocusLost>() && state == GameState::Playing)
        {
            state = GameState::Paused;
            menu_index = 0;
        }
    }
}

// Moves everything in the world and checks who hits whom
void Engine::update()
{
    // Menus and pause: nothing in the world moves
    if (state != GameState::Playing)
    {
        clock.restart();   // so there is no giant time jump when we resume
        return;
    }

    // delta_time = seconds since the last frame (capped so a lag spike can't break physics)
    float delta_time = clock.restart().asSeconds();
    if (delta_time > 0.05f)
        delta_time = 0.05f;

    player.update(delta_time, level.getPlatforms(), level.getWidth());

    // Gives enemies a way to add rocks, fireballs and spikes to the game
    EnemyWorld world{ projectiles, spikes };

    // ---- Enemies ----
    for (auto& enemy : enemies)
    {
        enemy->update(delta_time, level.getPlatforms(), player.getCenter(), world);
        if (!enemy->isAlive())
            continue;

        // Player strike hits enemy (once per swing)
        if (player.isStrikeActive()
            && player.getStrikeHitbox().findIntersection(enemy->getBounds())
            && enemy->getLastHitStrikeId() != player.getStrikeId())
        {
            enemy->takeDamage(1, player.getStrikeId(), player.getFacing());
        }

        // Which way to push the player: away from the enemy
        float push_dir = (player.getCenter().x < enemy->getCenter().x) ? -1.f : 1.f;

        // Touching the enemy's body
        if (enemy->hurtsOnContact()
            && player.getBounds().findIntersection(enemy->getBounds()))
        {
            player.takeDamage(1, push_dir);
        }

        // The enemy's attack (claws, explosion)
        sf::FloatRect attack_box;
        if (enemy->getAttackHitbox(attack_box)
            && player.getBounds().findIntersection(attack_box))
        {
            player.takeDamage(1, push_dir);
        }
    }

    // ---- Rocks and fireballs ----
    for (auto& projectile : projectiles)
    {
        projectile.update(delta_time, level.getPlatforms());
        if (projectile.isAlive() && projectile.getBounds().findIntersection(player.getBounds()))
        {
            float push_dir = (player.getCenter().x < projectile.getCenter().x) ? -1.f : 1.f;
            player.takeDamage(projectile.getDamage(), push_dir);
            projectile.kill();   // it breaks when it hits the player
        }
    }

    // ---- Stone spikes ----
    for (auto& spike : spikes)
    {
        spike.update(delta_time);
        if (spike.isDangerous() && spike.getBounds().findIntersection(player.getBounds()))
        {
            sf::FloatRect spike_box = spike.getBounds();
            float push_dir = (player.getCenter().x < spike_box.position.x + spike_box.size.x / 2.f) ? -1.f : 1.f;
            player.takeDamage(1, push_dir);
        }
    }

    // ---- Remove dead things ----
    // We go from the back to the front so erasing never skips an item
    for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; i--)
    {
        if (!enemies[i]->isAlive())
            enemies.erase(enemies.begin() + i);
    }
    for (int i = static_cast<int>(projectiles.size()) - 1; i >= 0; i--)
    {
        if (!projectiles[i].isAlive())
            projectiles.erase(projectiles.begin() + i);
    }
    for (int i = static_cast<int>(spikes.size()) - 1; i >= 0; i--)
    {
        if (!spikes[i].isAlive())
            spikes.erase(spikes.begin() + i);
    }

    camera.follow(player.getCenter(), level.getWidth(), level.getTop(), level.getHeight());
}

// The hearts in the top-left corner (drawn with the normal view so the camera doesn't move them)
void Engine::drawHUD()
{
    window.setView(window.getDefaultView());

    for (int i = 0; i < player.getMaxHealth(); i++)
    {
        // Full heart for health we have, empty heart for health we lost
        sf::Sprite health_sprite(i < player.getHealth() ? health_full_texture : health_empty_texture);

        health_sprite.setPosition({ 20.f + i * 40.f, 20.f }); // top-left corner of the window

        // Resize each sprite to 48x48
        sf::Vector2u texture_size = health_sprite.getTexture().getSize();
        health_sprite.setScale({ 48.f / texture_size.x, 48.f / texture_size.y }); // scale factor

        window.draw(health_sprite);
    }
}

void Engine::render()
{
    // Main menu: just the title and buttons
    if (state == GameState::MainMenu)
    {
        window.clear(sf::Color::Black);
        drawMenu("ORTHODOXY");
        window.display();
        return;
    }

    window.clear(sf::Color::Blue);
    camera.apply(window);

    // Draw order = back to front
    level.draw(window);

    for (auto& spike : spikes)
        spike.draw(window);

    for (auto& enemy : enemies)
        enemy->draw(window);

    for (auto& projectile : projectiles)
        projectile.draw(window);

    player.draw(window);
    drawHUD();

    // The pause menu is drawn on top of the frozen game
    if (state == GameState::Paused)
        drawMenu("PAUSED");

    window.display();
}
