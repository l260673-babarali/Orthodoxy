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

// Golem ka code ab Enemy.h mein hai.
// Big slow enemy: throws rocks from far away and smashes the ground (spikes) up close
class Golem : public Enemy
{
private:
    enum class State { Idle, Walk, ThrowWindup, SmashWindup, Recover };
    State state = State::Idle;
    float state_timer = 0.f;            // how long we have been in the current state
    float throw_cooldown = 1.f;         // seconds until the next rock throw is allowed
    float smash_cooldown = 0.f;         // seconds until the next ground smash is allowed
    bool last_attack_was_smash = false;   // decides which picture to show while recovering

    static constexpr float walk_speed = 45.f;       // very slow
    static constexpr float sight_range = 800.f;     // starts attacking from this far
    static constexpr float throw_min_range = 260.f;  // throws rocks when the player is at least this far
    static constexpr float smash_range = 180.f;     // smashes the floor when the player is this close

    // ---- Graphics ----
    sf::Texture texture;       // must come BEFORE the sprite
    sf::Sprite sprite;
    float anim_timer = 0.f;
    static constexpr int cell_width = 260;              // one frame in golem_sheet.png
    static constexpr int cell_height = 210;
    static constexpr float feet_y = 204.f;          // where the feet touch the bottom of a frame
    static constexpr float sprite_scale = 0.7f;

    void throwRock(sf::Vector2f player_center, EnemyWorld& world);
    void smashGround(EnemyWorld& world);
    void updateAnimation();

public:
    Golem(float x, float y);
    void update(float delta_time, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f player_center, EnemyWorld& world) override;
    void draw(sf::RenderWindow& window) override;
};

// BurstSpirit ka code ab Enemy.h mein hai.
// A floating spirit that chases the player and explodes if the player stands still near it
class BurstSpirit : public Enemy
{
private:
    enum class State { Chase, Fuse, Exploding };
    State state = State::Chase;
    float state_timer = 0.f;     // how long we have been in the current state
    float bob_timer = 0.f;       // drives the up/down floating
    bool noticed = false;       // becomes true once the player gets close enough to be seen

    // Used to find out if the player stopped moving
    sf::Vector2f last_player_center;
    bool has_last_player_pos = false;
    float still_timer = 0.f;     // how long the player has been standing still

    static constexpr float chase_speed = 150.f;       // player walks at 200, so you can outrun it
    static constexpr float sight_range = 600.f;
    static constexpr float trigger_range = 130.f;     // must be this close to start the countdown
    static constexpr float still_time_needed = 0.5f;   // player must stand still this long
    static constexpr float fuse_time = 0.6f;          // flashing before the boom
    static constexpr float explosion_time = 0.2f;
    static constexpr float explosion_radius = 110.f;

public:
    BurstSpirit(float x, float y);

    void update(float delta_time, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f player_center, EnemyWorld& world) override;
    void takeDamage(int amount, int strike_id, float from_direction) override;
    bool hurtsOnContact() const override { return false; }
    bool getAttackHitbox(sf::FloatRect& box) const override;
};

// Phoenix ka code ab Enemy.h mein hai.
// Flying enemy: hovers near the player, then breathes fire (far) or swoops with claws (close)
class Phoenix : public Enemy
{
private:
    enum class State { Hover, BreathWindup, Breathing, ClawWindup, ClawStrike, Recover, Dying };
    State state = State::Hover;
    float state_timer = 0.f;        // how long we have been in the current state
    float attack_cooldown = 1.5f;   // seconds until the next attack
    float breath_shot_timer = 0.f;   // countdown between two fireballs
    float breath_angle = 0.f;       // direction of the fire (picked once, when the breath starts)
    sf::Vector2f lunge_dir = { 1.f, 0.f };   // direction of the claw swoop

    static constexpr float hover_speed = 170.f;
    static constexpr float hover_distance = 300.f;  // how far away it likes to stay
    static constexpr float hover_height = 130.f;    // how high above the player
    static constexpr float claw_range = 220.f;      // closer than this = claws
    static constexpr float claw_lunge_speed = 420.f;
    static constexpr float dying_time = 0.8f;       // length of the death animation

    // ---- Graphics ----
    sf::Texture texture;       // must come BEFORE the sprite
    sf::Sprite sprite;
    float anim_timer = 0.f;
    static constexpr int frame_width = 140;             // one frame in the sheet
    static constexpr int frame_height = 139;
    static constexpr float sprite_scale = 0.7f;

    void hoverNear(sf::Vector2f player_center, float delta_time);
    void updateAnimation();

public:
    Phoenix(float x, float y);
    void update(float delta_time, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f player_center, EnemyWorld& world) override;
    void draw(sf::RenderWindow& window) override;
    void takeDamage(int amount, int strike_id, float from_direction) override;
    bool hurtsOnContact() const override { return false; }
    bool getAttackHitbox(sf::FloatRect& box) const override;
};

