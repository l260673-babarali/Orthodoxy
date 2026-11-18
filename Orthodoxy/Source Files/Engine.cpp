#include "../Classes/GameplayExtras.h"
#include "../Classes/Engine.h"
#include <iostream>

namespace
{
    const sf::Color ink(12, 11, 16);
    const sf::Color panel(23, 20, 27, 232);
    const sf::Color border(116, 87, 69);
    const sf::Color gold(223, 187, 123);
    const sf::Color parchment(232, 222, 201);
    const sf::Color muted(174, 160, 151);
    const sf::Color crimson(145, 51, 57);

    void uiPanel(sf::RenderWindow& window, float x, float y, float w, float h)
    {
        sf::RectangleShape shadow({ w + 8.f, h + 8.f });
        shadow.setPosition({ x + 5.f, y + 7.f });
        shadow.setFillColor(sf::Color(0, 0, 0, 115));
        window.draw(shadow);
        sf::RectangleShape surface({ w, h });
        surface.setPosition({ x, y });
        surface.setFillColor(panel);
        surface.setOutlineThickness(1.f);
        surface.setOutlineColor(border);
        window.draw(surface);
        sf::RectangleShape accent({ w - 20.f, 2.f });
        accent.setPosition({ x + 10.f, y + 9.f });
        accent.setFillColor(crimson);
        window.draw(accent);
    }

    void uiText(sf::RenderWindow& window, const sf::Font& font,
        const std::string& value, unsigned int size, float x, float y,
        sf::Color color, bool centered = false)
    {
        sf::Text text(font, value, size);
        text.setFillColor(color);
        text.setOutlineColor(sf::Color(0, 0, 0, 140));
        text.setOutlineThickness(0.6f);
        if (centered)
        {
            sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y });
        }
        text.setPosition({ x, y });
        window.draw(text);
    }

    void uiRule(sf::RenderWindow& window, float x, float y, float width)
    {
        sf::RectangleShape rule({ width, 1.f });
        rule.setPosition({ x, y });
        rule.setFillColor(border);
        window.draw(rule);
    }

    std::string stageClock(float seconds)
    {
        int elapsed = static_cast<int>(seconds);
        int minutes = elapsed / 60;
        int remainder = elapsed % 60;
        return std::to_string(minutes) + ":" +
            (remainder < 10 ? "0" : "") + std::to_string(remainder);
    }
}


Engine::Engine()
    : window(sf::VideoMode({ 1280u, 720u }), "Orthodoxy")
{
    ui_view.setSize({ 960.f, 540.f });
    ui_view.setCenter({ 480.f, 270.f });

    Extras::load();

    if (!health_full_texture.loadFromFile("Assets/Sprites/Health/Full.png"))
        std::cout << "Could not load Full.png\n";
    if (!health_empty_texture.loadFromFile("Assets/Sprites/Health/Empty.png"))
        std::cout << "Could not load Empty.png\n";

    font_loaded = font.openFromFile("C:/Windows/Fonts/georgia.ttf")
        || font.openFromFile("C:/Windows/Fonts/constan.ttf")
        || font.openFromFile("Assets/Fonts/font.ttf")
        || font.openFromFile("C:/Windows/Fonts/arial.ttf");
    title_font_loaded = title_font.openFromFile("C:/Windows/Fonts/georgiab.ttf")
        || title_font.openFromFile("C:/Windows/Fonts/georgia.ttf")
        || title_font.openFromFile("C:/Windows/Fonts/arialbd.ttf");
    if (!font_loaded)
        std::cout << "UI font could not be loaded!\n";
    if (!title_font_loaded)
        std::cout << "Title font could not be loaded!\n";
}

Engine::~Engine()
{
    clearEnemies();
}

void Engine::clearEnemies()
{
    for (Enemy* enemy : enemies)
        delete enemy;
    enemies.clear();
}

void Engine::startGame()
{
    blinking = false;
    total_kills = 0;
    best_combo = 0;
    combo_hits = 0;
    combo_timer = 0.f;
    level.reset();
    enterRegion(Region::AshenBarrow);
    state = GameState::Playing;
}

// Enemies for whichever region is loaded right now
void Engine::spawnEnemies()
{
    clearEnemies();
    projectiles.clear();
    spikes.clear();

    if (level.getRegion() == Region::AshenBarrow)
    {
        enemies.push_back(new Golem(1000.f, 430.f));
        enemies.push_back(new BurstSpirit(1450.f, 380.f));
        enemies.push_back(new BurstSpirit(1650.f, 360.f));
        enemies.push_back(new Phoenix(1950.f, 250.f));
        enemies.push_back(new Skeleton(2700.f, 780.f));            // falls onto the long floor
    }
    else if (level.getRegion() == Region::RedMoonRuins)
    {
        // Positions are in world coordinates (scene5 starts at x=0, scene6 at 1940, scene7 at 3880)
        enemies.push_back(new Golem(900.f, 885.f));                 // scene5, bottom middle ground
        enemies.push_back(new BurstSpirit(1700.f, 700.f));          // scene5, right side
        enemies.push_back(new BurstSpirit(2700.f, 650.f));          // scene6
        enemies.push_back(new Phoenix(3300.f, 400.f));              // scene6, flying
        enemies.push_back(new Golem(4780.f, 825.f));                // scene7, bottom middle ground
        enemies.push_back(new Skeleton(550.f, 780.f));              // scene5, left floor
        enemies.push_back(new Skeleton(2900.f, 840.f));             // scene6, ground
    }
    // The cathedral has no enemies yet
}

// Load a region: build its level, add its enemies, put the player at its start
void Engine::enterRegion(Region region)
{
    level.setRegion(region);
    stage_kills = 0;
    stage_relics = 0;
    stage_seconds = 0.f;
    combo_hits = 0;
    combo_timer = 0.f;
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

    window.setView(ui_view);

    float lidHeight = 270.f * amount;
    sf::RectangleShape topLid({ 960.f, lidHeight });
    topLid.setFillColor(sf::Color::Black);
    window.draw(topLid);

    sf::RectangleShape bottomLid({ 960.f, lidHeight });
    bottomLid.setPosition({ 0.f, 540.f - lidHeight });
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
        // Show results before entering the next region.
        if (key == K::E && !blinking && Extras::nearDoor(player.getCenter(), level))
        {
            cleared_region = level.getRegion();
            state = GameState::StageCleared;
            return;
        }

        if (key == K::Escape)
        {
            state = GameState::Paused;
            menu_index = 0;
        }
        return;
    }

    if (state == GameState::StageCleared)
    {
        if (key == K::Enter || key == K::Space || key == K::E)
        {
            Extras::enter();
            startBlink(level.getNextRegion());
            state = GameState::Playing;
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
    window.setView(ui_view);

    // Use the level behind pause, but a dark ornamental backdrop on the title screen.
    if (state == GameState::Paused)
    {
        sf::RectangleShape dim({ 960.f, 540.f });
        dim.setFillColor(sf::Color(7, 5, 11, 210));
        window.draw(dim);
    }
    else
    {
        sf::RectangleShape backdrop({ 960.f, 540.f });
        backdrop.setFillColor(ink);
        window.draw(backdrop);
        sf::RectangleShape redGlow({ 960.f, 190.f });
        redGlow.setPosition({ 0.f, 350.f });
        redGlow.setFillColor(sf::Color(59, 23, 34, 110));
        window.draw(redGlow);
    }
    if (!font_loaded)
        return;

    const sf::Font& headingFont = title_font_loaded ? title_font : font;
    uiPanel(window, 270.f, 89.f, 420.f, 346.f);
    uiText(window, font, state == GameState::Paused ? "ALTAR OF REFLECTION" :
        "A DARK FANTASY ACTION PLATFORMER", 13, 480.f, 111.f, muted, true);
    uiText(window, headingFont, title, state == GameState::Paused ? 40 : 49,
        480.f, 143.f, gold, true);
    uiRule(window, 346.f, 199.f, 268.f);

    std::vector<std::string> items = menuItems();
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        bool selected = i == menu_index;
        const float y = 227.f + i * 49.f;
        if (selected)
        {
            sf::RectangleShape focus({ 272.f, 38.f });
            focus.setPosition({ 344.f, y - 4.f });
            focus.setFillColor(sf::Color(80, 31, 40, 165));
            focus.setOutlineThickness(1.f);
            focus.setOutlineColor(crimson);
            window.draw(focus);
            uiText(window, headingFont, "◆", 15, 360.f, y + 3.f, gold);
        }
        uiText(window, font, items[i], 23, 480.f, y,
            selected ? gold : parchment, true);
    }
    uiText(window, font, "W / S  -  SELECT       ENTER  -  CONFIRM",
        12, 480.f, 407.f, muted, true);
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

    stage_seconds += delta_time;
    if (combo_timer > 0.f)
    {
        combo_timer -= delta_time;
        if (combo_timer <= 0.f)
            combo_hits = 0;
    }

    player.update(delta_time, level.getPlatforms(), level.getWidth());
    int health_before_combat = player.getHealth();
    Extras::updateSound(Extras::nearDoor(player.getCenter(), level));

    EnemyWorld world{ projectiles, spikes };

    for (Enemy* enemy : enemies)
    {
        enemy->update(delta_time, level.getPlatforms(), player.getCenter(), world);
        if (!enemy->isAlive())
            continue;

        if (player.isStrikeActive()
            && player.getStrikeHitbox().findIntersection(enemy->getBounds())
            && enemy->getLastHitStrikeId() != player.getStrikeId())
        {
            enemy->takeDamage(1, player.getStrikeId(), player.getFacing());
            if (enemy->getLastHitStrikeId() == player.getStrikeId())
                recordSwordHit();
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
        {
            if (enemies[i]->wasDefeatedByPlayer())
            {
                ++stage_kills;
                ++total_kills;
            }
            delete enemies[i];
            enemies.erase(enemies.begin() + i);
        }
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

    if (player.getHealth() < health_before_combat)
    {
        combo_hits = 0;
        combo_timer = 0.f;
    }
    camera.follow(player.getCenter(), level.getWidth(), level.getTop(), level.getHeight());
}

void Engine::recordSwordHit()
{
    ++combo_hits;
    if (combo_hits > best_combo)
        best_combo = combo_hits;
    combo_timer = combo_window;
}

std::string Engine::currentObjective() const
{
    if (level.getRegion() == Region::AshenBarrow)
        return "Reach the gate to Red Moon Ruins";
    if (level.getRegion() == Region::RedMoonRuins)
        return "Find the portal to the Cathedral";
    return "Explore the Cathedral"; // Boss objective comes in Phase 4
}

void Engine::drawHUD()
{
    window.setView(ui_view);
    // Small, low-profile panels leave more room for the scenery and enemies.
    uiPanel(window, 12.f, 10.f, 334.f, 100.f);
    uiPanel(window, 805.f, 10.f, 143.f, 60.f);

    for (int i = 0; i < player.getMaxHealth(); ++i)
    {
        sf::Sprite heart(i < player.getHealth() ? health_full_texture : health_empty_texture);
        heart.setPosition({ 22.f + i * 29.f, 22.f });
        sf::Vector2u size = heart.getTexture().getSize();
        if (size.x > 0 && size.y > 0)
            heart.setScale({ 25.f / size.x, 25.f / size.y });
        window.draw(heart);
    }
    if (!font_loaded)
        return;

    uiText(window, font, "KILLS", 11, 22.f, 53.f, muted);
    uiText(window, font, std::to_string(stage_kills) + " STAGE  /  " +
        std::to_string(total_kills) + " TOTAL", 15, 74.f, 50.f, gold);
    uiRule(window, 22.f, 76.f, 311.f);
    uiText(window, font, currentObjective(), 12, 22.f, 81.f, parchment);

    uiText(window, font, "STAGE TIME", 11, 818.f, 20.f, muted);
    uiText(window, title_font_loaded ? title_font : font,
        stageClock(stage_seconds), 21, 818.f, 36.f, gold);

    if (combo_hits > 1)
    {
        uiPanel(window, 805.f, 80.f, 143.f, 54.f);
        uiText(window, font, "BLADE COMBO", 11, 818.f, 89.f, muted);
        uiText(window, title_font_loaded ? title_font : font,
            "x" + std::to_string(combo_hits), 22, 818.f, 106.f, gold);
    }
}

void Engine::drawStageCleared()
{
    window.setView(ui_view);
    window.clear(ink);
    sf::RectangleShape glow({ 960.f, 215.f });
    glow.setPosition({ 0.f, 325.f });
    glow.setFillColor(sf::Color(67, 23, 38, 80));
    window.draw(glow);
    if (!font_loaded)
        return;

    const sf::Font& headingFont = title_font_loaded ? title_font : font;
    uiPanel(window, 232.f, 82.f, 496.f, 375.f);
    uiText(window, font, "THE PENANCE CONTINUES", 13, 480.f, 102.f, muted, true);
    uiText(window, headingFont, "STAGE CLEARED", 38, 480.f, 126.f, gold, true);
    std::string area = cleared_region == Region::AshenBarrow
        ? "ASHEN BARROW" : "RED MOON RUINS";
    uiText(window, headingFont, area, 19, 480.f, 181.f, parchment, true);
    uiRule(window, 270.f, 215.f, 420.f);

    const float left = 284.f, right = 524.f;
    uiText(window, font, "TIME TAKEN", 12, left, 232.f, muted);
    uiText(window, headingFont, stageClock(stage_seconds), 24, left, 248.f, gold);
    uiText(window, font, "UNDEAD BANISHED", 12, right, 232.f, muted);
    uiText(window, headingFont, std::to_string(stage_kills), 24, right, 248.f, gold);

    uiText(window, font, "BEST COMBO", 12, left, 298.f, muted);
    uiText(window, headingFont, "x" + std::to_string(best_combo), 24,
        left, 314.f, parchment);
    uiText(window, font, "TOTAL UNDEAD", 12, right, 298.f, muted);
    uiText(window, headingFont, std::to_string(total_kills), 24,
        right, 314.f, parchment);
    uiRule(window, 270.f, 354.f, 420.f);
    // Relics become collectible in Phase 2.
    uiText(window, font, "RELICS COLLECTED  " + std::to_string(stage_relics) +
        "   (Phase 2)", 13, 480.f, 367.f, muted, true);
    uiText(window, font, "PRESS ENTER TO CONTINUE", 18, 480.f,
        416.f, gold, true);
}

void Engine::render()
{
    if (state == GameState::StageCleared)
    {
        drawStageCleared();
        window.display();
        return;
    }
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

    for (Enemy* enemy : enemies)
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




