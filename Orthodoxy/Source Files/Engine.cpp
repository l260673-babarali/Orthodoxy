#include "../Classes/GameplayExtras.h"
#include "../Classes/Engine.h"
#include <iostream>

Engine::Engine()
    : window(sf::VideoMode({ 800u, 600u }), "Orthodoxy")
{
    Extras::load();

    if (!health_full_texture.loadFromFile("Assets/Sprites/Health/Full.png"))
        std::cout << "Could not load Full.png\n";
    if (!health_empty_texture.loadFromFile("Assets/Sprites/Health/Empty.png"))
        std::cout << "Could not load Empty.png\n";

    font_loaded = font.openFromFile("Assets/Fonts/font.ttf")
        || font.openFromFile("C:/Windows/Fonts/arial.ttf");
    if (!font_loaded)
        std::cout << "FONT FAILED TO LOAD!" << std::endl;
}

void Engine::startGame()
{
    blinking = false;
    level.reset();
    enterRegion(Region::AshenBarrow);
    state = GameState::Playing;
}

// Enemies for whichever region is loaded right now
void Engine::spawnEnemies()
{
    enemies.clear();
    projectiles.clear();
    spikes.clear();

    if (level.getRegion() == Region::AshenBarrow)
    {
        enemies.push_back(std::make_unique<Golem>(1000.f, 430.f));
        enemies.push_back(std::make_unique<BurstSpirit>(1450.f, 380.f));
        enemies.push_back(std::make_unique<BurstSpirit>(1650.f, 360.f));
        enemies.push_back(std::make_unique<Phoenix>(1950.f, 250.f));
        enemies.push_back(std::make_unique<Skeleton>(2700.f, 780.f));            // falls onto the long floor
    }
    else if (level.getRegion() == Region::RedMoonRuins)
    {
        // Positions are in world coordinates (scene5 starts at x=0, scene6 at 1940, scene7 at 3880)
        enemies.push_back(std::make_unique<Golem>(900.f, 885.f));                 // scene5, bottom middle ground
        enemies.push_back(std::make_unique<BurstSpirit>(1700.f, 700.f));          // scene5, right side
        enemies.push_back(std::make_unique<BurstSpirit>(2700.f, 650.f));          // scene6
        enemies.push_back(std::make_unique<Phoenix>(3300.f, 400.f));              // scene6, flying
        enemies.push_back(std::make_unique<Golem>(4780.f, 825.f));                // scene7, bottom middle ground
        enemies.push_back(std::make_unique<Skeleton>(550.f, 780.f));              // scene5, left floor
        enemies.push_back(std::make_unique<Skeleton>(2900.f, 840.f));             // scene6, ground
    }
    // The cathedral has no enemies yet
}

// Load a region: build its level, add its enemies, put the player at its start
void Engine::enterRegion(Region region)
{
    level.setRegion(region);
    spawnEnemies();
    player.reset(level.getSpawn());
    camera.follow(player.getCenter(), level.getWidth(), level.getTop(), level.getHeight());
}

// ---- Blink transition ----
void Engine::startBlink(Region target)
{
    blinking = true;
    blinkSwitched = false;
    blinkTimer = 0.f;
    blinkTarget = target;
}

void Engine::updateBlink(float deltaTime)
{
    blinkTimer += deltaTime;

    // Eyes are fully closed: change the region now, the player can't see it happen
    if (!blinkSwitched && blinkTimer >= blinkCloseTime)
    {
        blinkSwitched = true;
        enterRegion(blinkTarget);
    }

    if (blinkTimer >= blinkCloseTime + blinkHoldTime + blinkOpenTime)
        blinking = false;
}

// Two black eyelids: one comes down from the top, one up from the bottom
void Engine::drawBlink()
{
    if (!blinking)
        return;

    float amount;   // 0 = eyes open, 1 = eyes closed
    if (blinkTimer < blinkCloseTime)
    {
        amount = blinkTimer / blinkCloseTime;
    }
    else if (blinkTimer < blinkCloseTime + blinkHoldTime)
    {
        amount = 1.f;
    }
    else
    {
        amount = 1.f - (blinkTimer - blinkCloseTime - blinkHoldTime) / blinkOpenTime;
    }
    amount = std::clamp(amount, 0.f, 1.f);
    amount = amount * amount * (3.f - 2.f * amount);   // smooth start and end

    window.setView(window.getDefaultView());

    float lidHeight = 300.f * amount;
    sf::RectangleShape topLid({ 800.f, lidHeight });
    topLid.setFillColor(sf::Color::Black);
    window.draw(topLid);

    sf::RectangleShape bottomLid({ 800.f, lidHeight });
    bottomLid.setPosition({ 0.f, 600.f - lidHeight });
    bottomLid.setFillColor(sf::Color::Black);
    window.draw(bottomLid);
}

std::vector<std::string> Engine::menuItems() const
{
    if (state == GameState::MainMenu)
        return { "Play", "Quit" };
    return { "Resume", "Main Menu", "Quit" };
}

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
            state = GameState::Playing;
        }
        else if (menu_index == 1)
        {
            state = GameState::MainMenu;
            menu_index = 0;
        }
        else
        {
            window.close();
        }
    }
}

void Engine::handleKey(sf::Keyboard::Key key)
{
    using K = sf::Keyboard::Key;

    if (state == GameState::Playing)
    {
        // Press E at a portal: blink, then the next region loads
        if (key == K::E && !blinking && Extras::nearDoor(player.getCenter(), level))
        {
            Extras::enter();
            startBlink(level.getNextRegion());
        }

        if (key == K::Escape)
        {
            state = GameState::Paused;
            menu_index = 0;
        }
        return;
    }

    int count = (int)(menuItems().size());
    if (key == K::W || key == K::Up)   menu_index = (menu_index + count - 1) % count;
    if (key == K::S || key == K::Down) menu_index = (menu_index + 1) % count;
    if (key == K::Enter || key == K::Space) activateMenuItem();
    if (key == K::Escape && state == GameState::Paused)
        state = GameState::Playing;
}

void Engine::drawMenu(const std::string& title)
{
    window.setView(window.getDefaultView());

    if (state == GameState::Paused)
    {
        sf::RectangleShape dim({ 800.f, 600.f });
        dim.setFillColor(sf::Color(0, 0, 0, 160));
        window.draw(dim);
    }

    if (!font_loaded)
        return;

    sf::Text title_text(font, title, 56);
    title_text.setFillColor(sf::Color(220, 190, 110));
    sf::FloatRect title_bounds = title_text.getLocalBounds();
    title_text.setOrigin({ title_bounds.position.x + title_bounds.size.x / 2.f, 0.f });
    title_text.setPosition({ 400.f, 120.f });
    window.draw(title_text);

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
        {
            handleKey(key->code);
        }

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouse->button == sf::Mouse::Button::Left)
            {
                // Camera ki view se poori level ki position milegi
                sf::Vector2f pos = window.mapPixelToCoords(
                    mouse->position, camera.getView()
                );

                std::cout << "World X: " << (int)pos.x
                    << "  World Y: " << (int)pos.y
                    << std::endl;
            }
        }

        // Game window focus lose kare to pause
        if (event->is<sf::Event::FocusLost>() &&
            state == GameState::Playing)
        {
            state = GameState::Paused;
            menu_index = 0;
        }
    }
}

void Engine::update()
{
    if (state != GameState::Playing)
    {
        clock.restart();
        return;
    }

    float delta_time = clock.restart().asSeconds();
    if (delta_time > 0.05f)
        delta_time = 0.05f;

    // While the eyes are blinking the world is frozen
    if (blinking)
    {
        updateBlink(delta_time);
        return;
    }

    player.update(delta_time, level.getPlatforms(), level.getWidth());
    Extras::updateSound(Extras::nearDoor(player.getCenter(), level));

    EnemyWorld world{ projectiles, spikes };

    for (auto& enemy : enemies)
    {
        enemy->update(delta_time, level.getPlatforms(), player.getCenter(), world);
        if (!enemy->isAlive())
            continue;

        if (player.isStrikeActive()
            && player.getStrikeHitbox().findIntersection(enemy->getBounds())
            && enemy->getLastHitStrikeId() != player.getStrikeId())
        {
            enemy->takeDamage(1, player.getStrikeId(), player.getFacing());
        }

        float push_dir = (player.getCenter().x < enemy->getCenter().x) ? -1.f : 1.f;

        if (enemy->hurtsOnContact()
            && player.getBounds().findIntersection(enemy->getBounds()))
        {
            player.takeDamage(1, push_dir);
        }

        sf::FloatRect attack_box;
        if (enemy->getAttackHitbox(attack_box)
            && player.getBounds().findIntersection(attack_box))
        {
            player.takeDamage(1, push_dir);
        }
    }

    for (auto& projectile : projectiles)
    {
        projectile.update(delta_time, level.getPlatforms());
        // While dodging, rocks and fireballs fly straight through the knight
        if (projectile.isAlive() && !player.isDodging()
            && projectile.getBounds().findIntersection(player.getBounds()))
        {
            float push_dir = (player.getCenter().x < projectile.getCenter().x) ? -1.f : 1.f;
            player.takeDamage(projectile.getDamage(), push_dir);
            projectile.kill();
        }
    }

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

void Engine::drawHUD()
{
    window.setView(window.getDefaultView());

    for (int i = 0; i < player.getMaxHealth(); i++)
    {
        sf::Sprite health_sprite(i < player.getHealth() ? health_full_texture : health_empty_texture);

        health_sprite.setPosition({ 20.f + i * 40.f, 20.f });

        sf::Vector2u texture_size = health_sprite.getTexture().getSize();
        health_sprite.setScale({ 48.f / texture_size.x, 48.f / texture_size.y });

        window.draw(health_sprite);
    }
}

void Engine::render()
{
    if (state == GameState::MainMenu)
    {
        window.clear(sf::Color::Black);
        drawMenu("ORTHODOXY");
        window.display();
        return;
    }

    window.clear(sf::Color::Black);
    camera.apply(window);

    level.draw(window);
    Extras::draw(window, level, !blinking && Extras::nearDoor(player.getCenter(), level),
        font, font_loaded);

    for (auto& spike : spikes)
        spike.draw(window);

    for (auto& enemy : enemies)
        enemy->draw(window);

    for (auto& projectile : projectiles)
        projectile.draw(window);

    player.draw(window);
    drawHUD();
    drawBlink();

    if (state == GameState::Paused)
        drawMenu("PAUSED");

    window.display();
}




