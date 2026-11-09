#include "Enemy.h"

Enemy::Enemy(sf::Vector2f position, sf::Vector2f size, sf::Color color, int startHealth)
    : baseColor(color), health(startHealth)
{
    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(color);
}

void Enemy::updateHurtFlash(float deltaTime)
{
    if (hurtTimer > 0.f)
    {
        hurtTimer -= deltaTime;
        shape.setFillColor(sf::Color::White);
    }
    else
    {
        shape.setFillColor(baseColor);
    }
}

// Returns how far to push the enemy this frame after being hit (0 if not hit)
float Enemy::consumeKnockback(float deltaTime)
{
    if (knockbackTimer <= 0.f)
        return 0.f;

    knockbackTimer -= deltaTime;
    return knockbackDir * knockbackSpeed * deltaTime;
}

// Walk sideways and stop at walls. Returns true if we hit a wall.
bool Enemy::moveHorizontal(float moveX, const std::vector<sf::RectangleShape>& solids)
{
    if (moveX == 0.f)
        return false;

    bool hitWall = false;
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
            hitWall = true;
        }
    }
    return hitWall;
}

// Fall down and stand on the floor
void Enemy::applyGravity(float deltaTime, const std::vector<sf::RectangleShape>& solids)
{
    const float gravity = 981.f;

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
    if (!alive)
        return;

    window.draw(shape);

    // Red see-through box = "this area can hurt you right now"
    sf::FloatRect box;
    if (getAttackHitbox(box))
    {
        sf::RectangleShape attackBox(box.size);
        attackBox.setPosition(box.position);
        attackBox.setFillColor(sf::Color(255, 0, 0, 100));
        window.draw(attackBox);
    }
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