#include "../Classes/Spike.h"
#include <algorithm>
#include <iostream>

namespace
{
    sf::Texture& spikeTexture()
    {
        static sf::Texture texture;
        static bool tried = false;
        if (!tried)
        {
            tried = true;
            if (!texture.loadFromFile("Assets/Sprites/Enemies/Golem/spikes_sheet.png"))
                std::cout << "Could not load spikes_sheet.png\n";
        }
        return texture;
    }
    constexpr int cell_width = 200;                  // one frame in spikes_sheet.png
    constexpr int cell_height = 190;
    constexpr float floor_in_frame = 186.f;     // where the floor is inside a frame
    constexpr float spike_scale = 0.5f;
}

Spike::Spike(float center_x, float floor_y, float wait_time)
    : x(center_x), ground_y(floor_y), delay(wait_time)
{
    shape.setSize({ width, 6.f });
    shape.setPosition({ x - width / 2.f, ground_y - 6.f });
    shape.setFillColor(sf::Color(120, 100, 80));
}

// A spike has 4 phases: wait (rubble) -> rise -> stay -> sink
void Spike::update(float delta_time)
{
    if (!alive)
        return;

    timer += delta_time;
    float time_since_start = timer - delay;   // negative while we are still waiting
    float height = 6.f;
    dangerous = false;
    frame = -1;

    if (time_since_start < 0.f)
    {
        // Waiting: only a little pile of rubble (a warning!)
        height = 6.f;
        shape.setFillColor(sf::Color(120, 100, 80));
    }
    else if (time_since_start < rise_time)
    {
        height = max_height * (time_since_start / rise_time);
        dangerous = true;
        frame = std::min(2, static_cast<int>(time_since_start / rise_time * 3.f));   // small -> medium -> big
    }
    else if (time_since_start < rise_time + stay_time)
    {
        height = max_height;
        dangerous = true;
        frame = 3;                                                    // full size
    }
    else if (time_since_start < rise_time + stay_time + sink_time)
    {
        float sink_progress = (time_since_start - rise_time - stay_time) / sink_time;   // 0 -> 1
        height = max_height * (1.f - sink_progress);
        dangerous = true;
        frame = (sink_progress < 0.5f) ? 4 : 5;                       // shrinking, then crumbling
    }
    else
    {
        alive = false;   // finished, the game will remove it
        return;
    }

    // Too small to hurt anyone
    if (height < 8.f)
        dangerous = false;

    // The hitbox grows and shrinks with the spike
    shape.setSize({ width, height });
    shape.setPosition({ x - width / 2.f, ground_y - height });
}

void Spike::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;

    const sf::Texture& texture = spikeTexture();
    if (frame >= 0 && texture.getSize().x > 0)
    {
        sf::Sprite sprite(texture);
        sprite.setTextureRect(sf::IntRect({ frame * cell_width, 0 }, { cell_width, cell_height }));
        sprite.setOrigin({ cell_width / 2.f, floor_in_frame });   // the bottom middle of the spikes
        sprite.setScale({ spike_scale, spike_scale });
        sprite.setPosition({ x, ground_y });
        window.draw(sprite);
    }
    else
    {
        window.draw(shape);   // the little rubble pile while waiting
    }
}

bool Spike::isAlive() const { return alive; }
bool Spike::isDangerous() const { return dangerous; }
sf::FloatRect Spike::getBounds() const { return shape.getGlobalBounds(); }
