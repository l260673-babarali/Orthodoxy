
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Player
{
private:
    sf::RectangleShape shape;

    float velocityY = 0.f;

    bool OnGround = false;

    // Air dash variables
    bool canDash = true;
    bool isDashing = false;

    float dashSpeed = 650.f;
    float dashDuration = 0.2f;
    float dashTimer = 0.f;

    // Direction the player is facing
    float facingDirection = 1.f;

public:
    Player();

    void update(
        float deltaTime,
        const std::vector<sf::RectangleShape>& platforms,
        float worldWidth
    );

    void draw(sf::RenderWindow& window);

    sf::Vector2f getCenter() const;
};
