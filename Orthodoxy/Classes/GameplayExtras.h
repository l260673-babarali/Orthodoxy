#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>
#include "Level.h"

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

    // Position of the gate picture in the Ashen Cemetery. The "Press E" area of every
    // region lives in Level::getDoorTrigger().
    float doorX()
    {
        return 3057.f;
    }

    float doorY()
    {
        return 95.f;
    }

    bool nearDoor(sf::Vector2f player, const Level& level)
    {
        if (!level.hasDoor())
            return false;

        // Player gate ke paas ho to E ka message dikhao.
        return level.getDoorTrigger().contains(player);
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
        // Fire animation ki frame speed 8 frames/second hai; time x8 aur % count se frames loop karte hain.
        int frame = int(animationClock.getElapsedTime().asSeconds() * 8.f) % count;
        sf::Sprite sprite(flameTexture);
        // Fire sprite-sheet crop: X = frame index x 256, Y = fire type row x 208.
        sprite.setTextureRect(sf::IntRect({ frame * 256, type * 208 }, { 256, 208 }));
        // Position is the bottom-middle of the effect.
        sprite.setOrigin({ 128.f, 208.f });
        sprite.setScale({ size, size });
        sprite.setPosition({ x, y });
        window.draw(sprite);
    }

    void draw(sf::RenderWindow& window, const Level& level, bool near, sf::Font& font, bool fontLoaded)
    {
        if (!level.hasDoor()) return;
        float x = doorX();
        float y = doorY();
        sf::Vector2f promptPosition;

        if (level.getRegion() == Region::Cemetery)
        {
            if (doorTexture.getSize().x != 0)
            {
                sf::Sprite gate(doorTexture);
                gate.setScale({ 0.9f, 0.9f });   // 173 x 230: fits the arch above the stairs
                gate.setPosition({ x, y });
                window.draw(gate);
            }

            // Gate ki dono taraf flames.
            // Pehli decorative flame portal se X+25 aur Y+190 offset par draw hoti hai.
            drawFire(window, x - 8.f, y + 230.f, 3, 0.40f);
            // Doosri flame same Y par X+168 offset par draw hoti hai.
            drawFire(window, x + 181.f, y + 230.f, 3, 0.40f);

            // Door bahut oopar hai; message visible rakhte hain.
            promptPosition = { x - 20.f, y - 40.f };
        }
        else
        {
            // Red Moon Ruins and the Cathedral: the portal is already painted in the last picture,
            // so we only show the message above it.
            sf::FloatRect trigger = level.getDoorTrigger();
            // Door prompt trigger ke left X se 10 px aage aur trigger Y se 30 px ooper draw hota hai.
            promptPosition = { trigger.position.x + 10.f, trigger.position.y - 30.f };
        }

        if (near && fontLoaded)
        {
            sf::Text prompt(font, "Press E to teleport", 23);
            prompt.setFillColor(sf::Color(255, 225, 145));
            prompt.setOutlineColor(sf::Color::Black);
            prompt.setOutlineThickness(2.f);
            prompt.setPosition(promptPosition);
            window.draw(prompt);
        }
    }
}
