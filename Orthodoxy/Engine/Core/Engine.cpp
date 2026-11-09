#include "Engine.h"
#include "../../Game/Enemies/Golem.h"
#include "../../Game/Enemies/BurstSpirit.h"
#include "../../Game/Enemies/Phoenix.h"

Engine::Engine()
    : window(sf::VideoMode({ 800u, 600u }), "Orthodoxy")
{
    // The floor is at y = 520, so a 90-tall golem stands at y = 430
    enemies.push_back(std::make_unique<Golem>(1000.f, 430.f));

    // Spirits float in the air
    enemies.push_back(std::make_unique<BurstSpirit>(1450.f, 380.f));
    enemies.push_back(std::make_unique<BurstSpirit>(1650.f, 360.f));

    // Phoenix flies high
    enemies.push_back(std::make_unique<Phoenix>(1950.f, 250.f));
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
    }
}

void Engine::update()
{
    float deltaTime = clock.restart().asSeconds();
    if (deltaTime > 0.05f)
        deltaTime = 0.05f;

    player.update(deltaTime, level.getPlatforms(), level.getWidth());

    EnemyWorld world{ projectiles, spikes };

    // ---- Enemies ----
    for (auto& enemy : enemies)
    {
        enemy->update(deltaTime, level.getPlatforms(), player.getCenter(), world);
        if (!enemy->isAlive())
            continue;

        // Player strike hits enemy (once per swing)
        if (player.isStrikeActive()
            && player.getStrikeHitbox().findIntersection(enemy->getBounds())
            && enemy->getLastHitStrikeId() != player.getStrikeId())
        {
            enemy->takeDamage(1, player.getStrikeId(), player.getFacing());
        }

        float pushDir = (player.getCenter().x < enemy->getCenter().x) ? -1.f : 1.f;

        // Touching the enemy's body
        if (enemy->hurtsOnContact()
            && player.getBounds().findIntersection(enemy->getBounds()))
        {
            player.takeDamage(1, pushDir);
        }

        // The enemy's attack (claws, explosion)
        sf::FloatRect attackBox;
        if (enemy->getAttackHitbox(attackBox)
            && player.getBounds().findIntersection(attackBox))
        {
            player.takeDamage(1, pushDir);
        }
    }

    // ---- Rocks and fireballs ----
    for (auto& p : projectiles)
    {
        p.update(deltaTime, level.getPlatforms());
        if (p.isAlive() && p.getBounds().findIntersection(player.getBounds()))
        {
            float pushDir = (player.getCenter().x < p.getCenter().x) ? -1.f : 1.f;
            player.takeDamage(p.getDamage(), pushDir);
            p.kill();
        }
    }

    // ---- Stone spikes ----
    for (auto& s : spikes)
    {
        s.update(deltaTime);
        if (s.isDangerous() && s.getBounds().findIntersection(player.getBounds()))
        {
            sf::FloatRect sb = s.getBounds();
            float pushDir = (player.getCenter().x < sb.position.x + sb.size.x / 2.f) ? -1.f : 1.f;
            player.takeDamage(1, pushDir);
        }
    }

    // ---- Clean up (loop backwards so erasing is safe) ----
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

    camera.follow(player.getCenter(), level.getWidth(), level.getTop());
}

void Engine::drawHUD()
{
    window.setView(window.getDefaultView()); // screen space, ignores camera

    for (int i = 0; i < player.getMaxHealth(); i++)
    {
        sf::RectangleShape heart({ 24.f, 24.f });
        heart.setPosition({ 20.f + i * 32.f, 20.f });

        if (i < player.getHealth())
            heart.setFillColor(sf::Color::Red);
        else
            heart.setFillColor(sf::Color(60, 0, 0));

        window.draw(heart);
    }
}

void Engine::render()
{
    window.clear(sf::Color::Blue);
    camera.apply(window);

    level.draw(window);

    for (auto& spike : spikes)
        spike.draw(window);
    for (auto& enemy : enemies)
        enemy->draw(window);
    for (auto& p : projectiles)
        p.draw(window);

    player.draw(window);
    drawHUD();
    window.display();
}