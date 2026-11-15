#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>

// Portal aur fire effects isi file mein rahenge.
namespace Extras
{
    sf::Texture doorTexture;
    sf::Texture flameTexture;
    sf::Clock animationClock;
    sf::Music fireSound;
    sf::Music doorSound;
    sf::Music enterSound;
    bool doorSoundPlaying = false;

    void load()
    {
        if (!doorTexture.loadFromFile("Assets/Sprites/Effects/portal.png"))
            std::cout << "Could not load portal.png\n";
        if (!flameTexture.loadFromFile("Assets/Sprites/Effects/flame.png"))
            std::cout << "Could not load flame.png\n";
        doorTexture.setSmooth(false);
        flameTexture.setSmooth(false);

        if (fireSound.openFromFile("Assets/Sounds/fire.wav"))
        {
            fireSound.setLooping(true);
            fireSound.setVolume(18.f);
            fireSound.play();
        }
        if (doorSound.openFromFile("Assets/Sounds/portal_hum.wav"))
        {
            doorSound.setLooping(true);
            doorSound.setVolume(35.f);
        }
        if (enterSound.openFromFile("Assets/Sounds/enter.wav"))
            enterSound.setVolume(55.f);
    }

    float doorX()
    {
        return 6284.f;
    }

    float doorY()
    {
        return 10.f;
    }

    bool nearDoor(sf::Vector2f player, bool cathedral)
    {
        if (cathedral)
            return false;

        float x = doorX();
        float y = doorY();

        // Player gate ke paas ho to E ka message dikhao.
        if (player.x > x - 100.f && player.x < x + 250.f)
        {
            if (player.y > y - 100.f && player.y < y + 900.f)
                return true;
        }
        return false;
    }

    void updateSound(bool near)
    {
        if (near && !doorSoundPlaying)
        {
            doorSound.play();
            doorSoundPlaying = true;
        }
        if (!near && doorSoundPlaying)
        {
            doorSound.stop();
            doorSoundPlaying = false;
        }
    }

    void enter()
    {
        doorSound.stop();
        doorSoundPlaying = false;
        enterSound.play();
    }

    // rows: 0 bonfire, 1 torch, 2 fireball, 3 eruption, 4 lava
    void drawFire(sf::RenderWindow& window, float x, float y, int type, float size)
    {
        if (flameTexture.getSize().x == 0) return;
        int count = (type == 2) ? 7 : 8;
        int frame = int(animationClock.getElapsedTime().asSeconds() * 8.f) % count;
        sf::Sprite sprite(flameTexture);
        sprite.setTextureRect(sf::IntRect({ frame * 256, type * 208 }, { 256, 208 }));
        // Position is the bottom-middle of the effect.
        sprite.setOrigin({ 128.f, 208.f });
        sprite.setScale({ size, size });
        sprite.setPosition({ x, y });
        window.draw(sprite);
    }

    void draw(sf::RenderWindow& window, float worldWidth, bool cathedral, bool near, sf::Font& font, bool fontLoaded)
    {
        if (cathedral) return;
        float x = doorX();
        float y = doorY();

        // Background mein flames
        drawFire(window, 1505.f, 810.f, 3, 0.60f);
        drawFire(window, 2545.f, 872.f, 3, 0.60f);
        drawFire(window, 2976.f, 513.f, 3, 0.25f);
        drawFire(window, 3232.f, 513.f, 3, 0.25f);
        drawFire(window, 4700.f, 565.f, 3, 0.60f);
        drawFire(window, 7145.f, 498.f, 3, 0.60f);
        drawFire(window, 7820.f, 540.f, 3, 0.60f);

        if (doorTexture.getSize().x != 0)
        {
            sf::Sprite gate(doorTexture);
            gate.setScale({ 1.3f, 1.3f });
            gate.setPosition({ x, y });
            window.draw(gate);
        }

        // Gate ki dono taraf flames.
        drawFire(window, x + 25.f, y + 190.f, 3, 0.40f);
        drawFire(window, x + 168.f, y + 190.f, 3, 0.40f);

        if (near && fontLoaded)
        {
            sf::Text prompt(font, "Press E to teleport", 23);
            prompt.setFillColor(sf::Color(255, 225, 145));
            prompt.setOutlineColor(sf::Color::Black);
            prompt.setOutlineThickness(2.f);
            // Door bahut oopar hai; message visible rakhte hain.
            prompt.setPosition({ x - 10.f, y + 440.f });
            window.draw(prompt);
        }
    }
}
