
#include "Player.h"

Player::Player()
{
    shape.setSize({ 50.f, 80.f });
    shape.setPosition({ 100.f, 100.f });
}

void Player::update(
    float deltaTime,
    const std::vector<sf::RectangleShape>& platforms,
    float worldWidth
)
{
    float speed = 200.f;
    float gravity = 981.f;
    float groundLevel = 520.f;
    float jumpVelocity = -400.f;

    // Horizontal movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        shape.move({ -speed * deltaTime, 0.f });
        facingDirection = -1.f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        shape.move({ speed * deltaTime, 0.f });
        facingDirection = 1.f;
    }

    // Jumping
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
        && OnGround)
    {
        velocityY = jumpVelocity;
        OnGround = false;
    }

    // Start air dash
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        && !OnGround && canDash)
    {
        isDashing = true;
        canDash = false;
        dashTimer = dashDuration;
        velocityY = 0.f;
    }

    // Dash movement
    if (isDashing)
    {
        shape.move({
            facingDirection * dashSpeed * deltaTime,
            0.f
            });

        dashTimer -= deltaTime;

        if (dashTimer <= 0.f)
        {
            isDashing = false;
        }
    }
    else
    {
        // Normal gravity
        velocityY += gravity * deltaTime;

        shape.move({
            0.f,
            velocityY * deltaTime
            });
    }

    // Remember previous bottom position
    float previousBottom =
        shape.getPosition().y + shape.getSize().y;

    // Ground and platform collision
    float currentBottom =
        shape.getPosition().y + shape.getSize().y;

    if (!isDashing && velocityY >= 0.f)
    {
        for (const auto& platform : platforms)
        {
            sf::FloatRect playerBounds = shape.getGlobalBounds();
            sf::FloatRect platformBounds = platform.getGlobalBounds();

            bool horizontalOverlap =
                playerBounds.position.x + playerBounds.size.x >
                platformBounds.position.x &&
                playerBounds.position.x <
                platformBounds.position.x + platformBounds.size.x;

            bool crossedPlatform =
                previousBottom <= platformBounds.position.y &&
                currentBottom >= platformBounds.position.y;

            if (horizontalOverlap && crossedPlatform)
            {
                shape.setPosition({
                    shape.getPosition().x,
                    platformBounds.position.y - shape.getSize().y
                    });

                velocityY = 0.f;
                OnGround = true;
                break;
            }
        }
    }

    // Ground collision
    if (shape.getPosition().y + shape.getSize().y >= groundLevel)
    {
        shape.setPosition({
            shape.getPosition().x,
            groundLevel - shape.getSize().y
            });

        velocityY = 0.f;
        OnGround = true;
    }

    // Reset dash when player touches the ground
    if (OnGround)
    {
        canDash = true;
    }

    // World boundaries
    if (shape.getPosition().x < 0.f)
    {
        shape.setPosition({ 0.f, shape.getPosition().y });
    }

    if (shape.getPosition().x + shape.getSize().x > worldWidth)
    {
        shape.setPosition({
            worldWidth - shape.getSize().x,
            shape.getPosition().y
            });
    }
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

sf::Vector2f Player::getCenter() const
{
    return {
        shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f
    };
}
