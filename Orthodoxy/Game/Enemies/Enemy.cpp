#include "Enemy.h"

Enemy::Enemy(float x, float y, float left, float right)
    : patrolLeft(left), patrolRight(right)
{
    shape.setSize({ 40.f, 40.f });
    shape.setPosition({ x, y });
    shape.setFillColor(sf::Color(180, 40, 40));
}

void Enemy::update(float deltaTime, const std::vector<sf::RectangleShape>& solids)
{
    if (!alive)
        return;

    const float gravity = 981.f;
    const float knockbackSpeed = 250.f;

    // Hurt flash
    if (hurtTimer > 0.f)
    {
        hurtTimer -= deltaTime;
        shape.setFillColor(sf::Color::White);
    }
    else
    {
        shape.setFillColor(sf::Color(180, 40, 40));
    }

    // ---- Horizontal movement ----
    float moveX = 0.f;
    if (knockbackTimer > 0.f)
    {
        moveX = knockbackDir * knockbackSpeed * deltaTime;
        knockbackTimer -= deltaTime;
    }
    else
    {
        moveX = direction * speed * deltaTime;
    }

    shape.move({ moveX, 0.f });

    for (const auto& solid : solids)
    {
        sf::FloatRect p = shape.getGlobalBounds();
        sf::FloatRect s = solid.getGlobalBounds();

        if (p.findIntersection(s))
        {
            if (moveX > 0.f)
                shape.setPosition({ s.position.x - p.size.x, p.position.y });
            else
                shape.setPosition({ s.position.x + s.size.x, p.position.y });

            // Hit a wall: turn around
            if (knockbackTimer <= 0.f)
                direction = -direction;
        }
    }

    // Patrol limits
    if (shape.getPosition().x < patrolLeft)
        direction = 1.f;
    if (shape.getPosition().x + shape.getSize().x > patrolRight)
        direction = -1.f;

    // ---- Vertical movement ----
    velocityY += gravity * deltaTime;
    shape.move({ 0.f, velocityY * deltaTime });

    for (const auto& solid : solids)
    {
        sf::FloatRect p = shape.getGlobalBounds();
        sf::FloatRect s = solid.getGlobalBounds();

        if (p.findIntersection(s))
        {
            if (velocityY > 0.f)
            {
                shape.setPosition({ p.position.x, s.position.y - p.size.y });
                velocityY = 0.f;
            }
            else if (velocityY < 0.f)
            {
                shape.setPosition({ p.position.x, s.position.y + s.size.y });
                velocityY = 0.f;
            }
        }
    }
}

void Enemy::draw(sf::RenderWindow& window)
{
    if (alive)
        window.draw(shape);
}

void Enemy::takeDamage(int amount, int strikeId, float fromDirection)
{
    health -= amount;
    lastHitStrikeId = strikeId;

    hurtTimer = 0.15f;
    knockbackTimer = 0.15f;
    knockbackDir = fromDirection;

    if (health <= 0)
        alive = false;
}

bool Enemy::isAlive() const { return alive; }
int Enemy::getLastHitStrikeId() const { return lastHitStrikeId; }
sf::FloatRect Enemy::getBounds() const { return shape.getGlobalBounds(); }

sf::Vector2f Enemy::getCenter() const
{
    return {
        shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f
    };
}