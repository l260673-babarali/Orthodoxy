#pragma once
#include <SFML/Graphics.hpp>

// A stone spike that pops out of the floor
class Spike
{
private:
    sf::RectangleShape shape;   // the hitbox (and the little rubble pile while waiting)
    float x;                    // middle of the spike
    float ground_y;              // top of the floor
    float delay;                // seconds to wait before popping out
    float timer = 0.f;
    bool alive = true;
    bool dangerous = false;     // can it hurt the player right now?
    int frame = -1;             // which picture of spikes_sheet.png (-1 = still waiting)

    static constexpr float width = 56.f;
    static constexpr float max_height = 80.f;
    static constexpr float rise_time = 0.10f;
    static constexpr float stay_time = 0.35f;
    static constexpr float sink_time = 0.15f;

public:
    Spike(float center_x, float floor_y, float wait_time);
    void update(float delta_time);
    void draw(sf::RenderWindow& window);
    bool isAlive() const;
    bool isDangerous() const;
    sf::FloatRect getBounds() const;
};
