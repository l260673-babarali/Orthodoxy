#include "../Classes/Enemy.h"

Enemy::Enemy(sf::Vector2f position, sf::Vector2f size, sf::Color color, int start_health)
    : base_color(color), health(start_health)
{
    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(color);
}

// Turn white for a moment after being hit
void Enemy::updateHurtFlash(float delta_time)
{
    if (hurt_timer > 0.f)
    {
        hurt_timer -= delta_time;
        shape.setFillColor(sf::Color::White);
    }
    else
    {
        shape.setFillColor(base_color);
    }
}

// Returns how far to push the enemy this frame after being hit (0 if not hit)
float Enemy::consumeKnockback(float delta_time)
{
    if (knockback_timer <= 0.f)
        return 0.f;

    knockback_timer -= delta_time;
    return knockback_dir * knockback_speed * delta_time;
}

// Walk sideways and stop at walls. Returns true if we hit a wall.
bool Enemy::moveHorizontal(float move_x, const std::vector<sf::RectangleShape>& solids)
{
    if (move_x == 0.f)
        return false;

    bool hit_wall = false;
    shape.move({ move_x, 0.f });

    // If we ended up inside a solid, push ourselves back out on the side we came from
    for (const auto& solid : solids)
    {
        sf::FloatRect enemy_box = shape.getGlobalBounds();
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (enemy_box.findIntersection(solid_box))
        {
            if (move_x > 0.f)   // moving right: stop at the solid's left side
                shape.setPosition({ solid_box.position.x - enemy_box.size.x, enemy_box.position.y });
            else               // moving left: stop at the solid's right side
                shape.setPosition({ solid_box.position.x + solid_box.size.x, enemy_box.position.y });
            hit_wall = true;
        }
    }
    return hit_wall;
}

// Fall down and stand on the floor
void Enemy::applyGravity(float delta_time, const std::vector<sf::RectangleShape>& solids)
{
    const float gravity = 981.f;

    velocity_y += gravity * delta_time;
    shape.move({ 0.f, velocity_y * delta_time });

    for (const auto& solid : solids)
    {
        sf::FloatRect enemy_box = shape.getGlobalBounds();
        sf::FloatRect solid_box = solid.getGlobalBounds();
        if (enemy_box.findIntersection(solid_box))
        {
            if (velocity_y > 0.f)   // falling: land on top of the solid
            {
                shape.setPosition({ enemy_box.position.x, solid_box.position.y - enemy_box.size.y });
                velocity_y = 0.f;
            }
            else if (velocity_y < 0.f)   // going up: bump the head on the underside
            {
                shape.setPosition({ enemy_box.position.x, solid_box.position.y + solid_box.size.y });
                velocity_y = 0.f;
            }
        }
    }
}

void Enemy::draw(sf::RenderWindow& window)
{
    if (!alive)
        return;

    window.draw(shape);

    // Blue = enemy body hitbox
    sf::RectangleShape hurtbox(shape.getSize());

    hurtbox.setPosition(shape.getPosition());
    hurtbox.setFillColor(sf::Color::Transparent);
    hurtbox.setOutlineColor(sf::Color::Cyan);
    hurtbox.setOutlineThickness(5.f);

    window.draw(hurtbox);

    // Red = active enemy attack hitbox
    sf::FloatRect box;
    if (getAttackHitbox(box))
    {
        sf::RectangleShape attack_box(box.size);
        attack_box.setPosition(box.position);
        attack_box.setFillColor(sf::Color(255, 0, 0, 100));

        window.draw(attack_box);
    }
}

void Enemy::takeDamage(int amount, int strike_id, float from_direction)
{
    health -= amount;
    last_hit_strike_id = strike_id;
    hurt_timer = 0.15f;
    knockback_timer = 0.15f;
    knockback_dir = from_direction;

    if (health <= 0)
        alive = false;
}

bool Enemy::isAlive() const { return alive; }
int Enemy::getLastHitStrikeId() const { return last_hit_strike_id; }
sf::FloatRect Enemy::getBounds() const { return shape.getGlobalBounds(); }

// The middle of the enemy's body
sf::Vector2f Enemy::getCenter() const
{
    return {
        shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f
    };
}
