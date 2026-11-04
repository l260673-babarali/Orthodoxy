#include "Engine.h"

Engine::Engine()
    : window(sf::VideoMode({ 800u, 600u }), "Orthodoxy")
{
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
    camera.follow(player.getCenter(), level.getWidth());
}

void Engine::render()
{
    window.clear(sf::Color::Blue);

    camera.apply(window);

    level.draw(window);
    player.draw(window);

    window.display();
}