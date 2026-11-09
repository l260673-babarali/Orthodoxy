#pragma once
#include <SFML/Graphics.hpp>

// A stone spike that pops out of the floor
class Spike
{
private:
    sf::RectangleShape shape;
    float x;        // middle of the spike
    float groundY;  // top of the floor
    float delay;    // seconds to wait before popping out
    float timer = 0.f;
    bool alive = true;
    bool dangerous = false;

    static constexpr float width = 44.f;
    static constexpr float maxHeight = 70.f;
    static constexpr float riseTime = 0.10f;
    static constexpr float stayTime = 0.35f;
    static constexpr float sinkTime = 0.15f;

public:
    Spike(float centerX, float floorY, float waitTime);

    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    bool isAlive() const;
    bool isDangerous() const;
    sf::FloatRect getBounds() const;
};