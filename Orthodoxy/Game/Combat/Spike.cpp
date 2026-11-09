#include "Spike.h"

Spike::Spike(float centerX, float floorY, float waitTime)
    : x(centerX), groundY(floorY), delay(waitTime)
{
    shape.setSize({ width, 6.f });
    shape.setPosition({ x - width / 2.f, groundY - 6.f });
    shape.setFillColor(sf::Color(120, 100, 80));
}

void Spike::update(float deltaTime)
{
    if (!alive)
        return;

    timer += deltaTime;
    float t = timer - delay;
    float height = 6.f;
    dangerous = false;

    if (t < 0.f)
    {
        // Waiting: only a little pile of rubble (a warning!)
        height = 6.f;
        shape.setFillColor(sf::Color(120, 100, 80));
    }
    else if (t < riseTime)
    {
        height = maxHeight * (t / riseTime);
        dangerous = true;
        shape.setFillColor(sf::Color(150, 140, 130));
    }
    else if (t < riseTime + stayTime)
    {
        height = maxHeight;
        dangerous = true;
        shape.setFillColor(sf::Color(150, 140, 130));
    }
    else if (t < riseTime + stayTime + sinkTime)
    {
        float sinkProgress = (t - riseTime - stayTime) / sinkTime;
        height = maxHeight * (1.f - sinkProgress);
        dangerous = true;
        shape.setFillColor(sf::Color(150, 140, 130));
    }
    else
    {
        alive = false;
        return;
    }

    if (height < 8.f)
        dangerous = false;

    shape.setSize({ width, height });
    shape.setPosition({ x - width / 2.f, groundY - height });
}

void Spike::draw(sf::RenderWindow& window)
{
    if (alive)
        window.draw(shape);
}

bool Spike::isAlive() const { return alive; }
bool Spike::isDangerous() const { return dangerous; }
sf::FloatRect Spike::getBounds() const { return shape.getGlobalBounds(); }