#include "Engine.h"

Engine::Engine()
    : window(sf::VideoMode({ 800u, 600u }), "Orthodoxy")
{
    // Enemy(x, y, patrolLeft, patrolRight)
    enemies.push_back(Enemy(1000.f, 470.f, 900.f, 1300.f));
    enemies.push_back(Enemy(1400.f, 470.f, 1300.f, 1700.f));
    enemies.push_back(Enemy(1900.f, 470.f, 1800.f, 2300.f));
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

    for (auto& enemy : enemies)
    {
        enemy.update(deltaTime, level.getPlatforms());

        if (!enemy.isAlive())
            continue;

        // Player strike hits enemy (once per swing)
        if (player.isStrikeActive()
            && player.getStrikeHitbox().findIntersection(enemy.getBounds())
            && enemy.getLastHitStrikeId() != player.getStrikeId())
        {
            enemy.takeDamage(1, player.getStrikeId(), player.getFacing());
        }

        // Enemy touches player
        if (player.getBounds().findIntersection(enemy.getBounds()))
        {
            float pushDir = (player.getCenter().x < enemy.getCenter().x) ? -1.f : 1.f;
            player.takeDamage(1, pushDir);
        }
    }

    // Remove dead enemies (loop backwards so erasing is safe)
    for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; i--)
    {
        if (!enemies[i].isAlive())
            enemies.erase(enemies.begin() + i);
    }

    camera.follow(player.getCenter(), level.getWidth());
}

void Engine::drawHUD()
{
    window.setView(window.getDefaultView());   // screen space, ignores camera

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

    for (auto& enemy : enemies)
        enemy.draw(window);

    player.draw(window);

    drawHUD();

    window.display();
}