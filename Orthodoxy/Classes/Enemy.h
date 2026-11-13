#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../Classes/Projectile.h"
#include "../Classes/Spike.h"

// Lets enemies put rocks, fireballs and spikes into the game
struct EnemyWorld
{
    std::vector<Projectile>& projectiles;
    std::vector<Spike>& spikes;
};

// Base class for every enemy (Golem, BurstSpirit, Phoenix)
class Enemy
{
protected:
    sf::RectangleShape shape;   // the enemy's body / hitbox
    sf::Color base_color;
    int health;
    bool alive = true;
    int last_hit_strike_id = -1;   // the last sword swing that hit us (so one swing hits only once)
    float hurt_timer = 0.f;      // while above 0 the enemy flashes
    float knockback_timer = 0.f;
    float knockback_dir = 0.f;
    float knockback_speed = 250.f;
    float velocity_y = 0.f;
    float facing = 1.f; // 1 = right, -1 = left

    void updateHurtFlash(float delta_time);
    float consumeKnockback(float delta_time);
    bool moveHorizontal(float move_x, const std::vector<sf::RectangleShape>& solids);
    void applyGravity(float delta_time, const std::vector<sf::RectangleShape>& solids);

public:
    Enemy(sf::Vector2f position, sf::Vector2f size, sf::Color color, int start_health);
    virtual ~Enemy() = default;

    virtual void update(float delta_time, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f player_center, EnemyWorld& world) = 0;
    virtual void draw(sf::RenderWindow& window);
    virtual void takeDamage(int amount, int strike_id, float from_direction);

    // Does just touching this enemy hurt the player?
    virtual bool hurtsOnContact() const { return true; }
    // Is the enemy hitting something right now (claws, explosion)? Fills 'box' if yes.
    virtual bool getAttackHitbox(sf::FloatRect&) const { return false; }

    bool isAlive() const;
    int getLastHitStrikeId() const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getCenter() const;
};
