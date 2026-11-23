#pragma once
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>

// Slash sounds and background music.
namespace GameAudio
{
    inline sf::SoundBuffer& buffer(int i)
    {
        static sf::SoundBuffer buffers[3];
        return buffers[i];
    }

    inline std::vector<sf::Sound>& voices()
    {
        static std::vector<sf::Sound> v;   // a few voices so quick slashes can overlap
        return v;
    }

    inline sf::Music& music()
    {
        static sf::Music m;
        return m;
    }

    inline void load()
    {
        static bool loaded = false;
        if (loaded) return;
        loaded = true;

        const char* files[3] = { "Assets/Sounds/slash_light1.wav", "Assets/Sounds/slash_light2.wav", "Assets/Sounds/slash_heavy.wav" };
        for (int i = 0; i < 3; i++)
            if (!buffer(i).loadFromFile(files[i]))
                std::cout << "Could not load " << files[i] << "\n";

        for (int i = 0; i < 6; i++)
            voices().push_back(sf::Sound(buffer(0)));

        if (music().openFromFile("Assets/Sounds/arcade_music.wav"))
        {
            music().setLooping(true);
            music().setVolume(20.f);
            music().play();
        }
        else
            std::cout << "Could not load arcade_music.wav\n";
    }

    // 0 / 1 = light slash (alternates), 2 = heavy slash
    inline void playSlash(int which)
    {
        load();
        for (sf::Sound& s : voices())
        {
            if (s.getStatus() != sf::Sound::Status::Playing)
            {
                s.setBuffer(buffer(which));
                s.setVolume(which == 2 ? 80.f : 60.f);
                s.setPitch(which == 2 ? 1.f : 0.95f + 0.1f * (float)(std::rand() % 100) / 100.f);
                s.play();
                return;
            }
        }
    }
}
