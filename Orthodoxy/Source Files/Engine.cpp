#include "../Classes/Engine.h"
#include "../Classes/Golem.h"
#include "../Classes/BurstSpirit.h"
#include "../Classes/Phoenix.h"


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

    healthFullTexture.loadFromFile("Assets/Sprites/Health/Full.png");
    healthEmptyTexture.loadFromFile("Assets/Sprites/Health/Empty.png");

    bool loaded = entranceBackgroundTexture.loadFromFile(
        "Assets/Sprites/Backgrounds/Entrance/monastery_entrance.jpg"
    );

    if (loaded)
    {
        entranceBackground = std::make_unique<sf::Sprite>(entranceBackgroundTexture);
        float scale = 0.9f;

        entranceBackground->setScale({ scale, scale });
    }
    else
    {
        std::cout << "BACKGROUND FAILED TO LOAD!" << std::endl;
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

    for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; i--)
    {
        if (!enemies[i]->isAlive())
            enemies.erase(enemies.begin() + i);
    }
    for (int i = static_cast<int>(projectiles.size()) - 1; i >= 0; i--)
    {
        if (!projectiles[i].isAlive())projectiles.erase(projectiles.begin() + i);
    }
    for (int i = static_cast<int>(spikes.size()) - 1; i >= 0; i--)
    {
        if (!spikes[i].isAlive()) spikes.erase(spikes.begin() + i);
    }

    camera.follow(player.getCenter(), level.getWidth(), level.getTop());
}

void Engine::drawHUD()
{
    window.setView(window.getDefaultView());

    for (int i = 0; i < player.getMaxHealth(); i++)
    {
        sf::Sprite healthSprite(i < player.getHealth()? healthFullTexture: healthEmptyTexture);

		healthSprite.setPosition({ 20.f + i * 40.f,20.f }); // this position is in the top-left corner of the window

        // Resize each sprite to 32x32
        sf::Vector2u textureSize = healthSprite.getTexture().getSize();

		healthSprite.setScale({ 48.f / textureSize.x,48.f / textureSize.y }); // dividing by texture size to get the scale factor

        window.draw(healthSprite);
    }
}

void Engine::render()
{
    window.clear(sf::Color::Blue);
    camera.apply(window);

    if (entranceBackground)
    {
        float backgroundHeight =
            entranceBackgroundTexture.getSize().y * 0.9f;

        float backgroundY =
            camera.getCenter().y - 300.f
            + (600.f - backgroundHeight);

        entranceBackground->setPosition({
            0.f,
            backgroundY
            });

        window.draw(*entranceBackground);
    }

    entranceBackground->setPosition({0.f, camera.getCenter().y - 300.f});

    window.draw(*entranceBackground);

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