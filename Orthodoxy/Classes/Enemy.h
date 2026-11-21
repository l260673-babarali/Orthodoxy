#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include "../Classes/Projectile.h"
#include "../Classes/Spike.h"

// Lets enemies put rocks, fireballs and spikes into the game
struct EnemyWorld
{
    std::vector<Projectile>& projectiles;
    std::vector<Spike>& spikes;
};

// Base class for every enemy (Golem, BurstSpirit, Phoenix, Skeleton, DraconicLord)
class Enemy
{
protected:
    sf::RectangleShape shape;   // the enemy's body / hitbox
    sf::Color base_color;
    int health;
    int max_health = 1;         // health at the start of the fight (for the boss health bar)
    int difficulty_level = 1;
    bool alive = true;
    bool defeated_by_player = false;  // excludes spirits that explode on their own
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

    // The boss has a health bar on the screen
    virtual bool isBoss() const { return false; }
    float getHealthFraction() const;   // 1 = full health, 0 = dead

    void setDifficulty(int level);
    int getAttackDamage() const;
    bool isAlive() const;
    bool wasDefeatedByPlayer() const;
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

// A simple skeleton (placeholder box for now, sprites come later).
// Close to the player: slashes with its weapon. Far from the player: does a quick jump attack.
class Skeleton : public Enemy
{
private:
    enum class State { Idle, Walk, SlashWindup, SlashStrike, LeapWindup, Leap, Recover };
    State state = State::Idle;
    float state_timer = 0.f;        // how long we have been in the current state
    float slash_cooldown = 0.5f;    // seconds until the next slash is allowed
    float leap_cooldown = 1.f;      // seconds until the next jump attack is allowed
    float leap_speed_x = 0.f;       // sideways speed chosen when the jump starts

    static constexpr float walk_speed = 70.f;
    static constexpr float sight_range = 700.f;       // notices the player from this far
    static constexpr float slash_range = 110.f;       // closer than this = slash
    static constexpr float leap_min_range = 180.f;    // farther than this = jump attack
    static constexpr float leap_max_range = 520.f;
    static constexpr float slash_windup_time = 0.45f; // warning before the slash (dash through it!)
    static constexpr float slash_active_time = 0.15f; // how long the slash can hurt
    static constexpr float leap_windup_time = 0.30f;  // crouch before the jump
    static constexpr float leap_jump_speed = 380.f;   // upward speed of the jump
    static constexpr float leap_max_speed_x = 500.f;
    static constexpr float recover_time = 0.6f;

public:
    Skeleton(float x, float y);

    void update(float delta_time, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f player_center, EnemyWorld& world) override;
    void draw(sf::RenderWindow& window) override;
    bool hurtsOnContact() const override { return false; }
    bool getAttackHitbox(sf::FloatRect& box) const override;
};

// The final boss: a draconic lord in human form (a plain box until the sprites are ready).
// He walks toward the player and uses 5 attacks, never the same one twice in a row:
//   0 = dragon scales (a ring of scales flies out in every direction, avoid it with a jump)
//   1 = draconic strike (a fast dash-slash across the room)
//   2 = fireball (one big ball of fire at the player)
//   3 = planted blade (a blade sticks in the ground, then explodes in a wide area)
//   4 = burning ground (the whole floor catches fire, stay in the air)
class DraconicLord : public Enemy
{
private:
    enum class State
    {
        Walk, ScalesWindup, StrikeWindup, StrikeDash, FireballWindup,
        BladeWindup, BladePlanted, BladeExplode, FireWindup, FireBurning, Recover
    };
    State state = State::Walk;
    float state_timer = 0.f;        // how long we have been in the current state
    float attack_cooldown = 2.f;    // seconds until the next attack (a short pause at the start)
    int last_attack = -1;           // so the same attack is not used twice in a row
    float blade_x = 0.f;            // where the blade is stuck in the ground
    float ground_y = 0.f;           // the top of the floor (where the boss stands)

    static constexpr float walk_speed = 90.f;
    static constexpr float stop_range = 250.f;       // stops walking when this close to the player
    static constexpr float attack_pause = 1.f;       // rest between two attacks
    static constexpr float recover_time = 0.7f;

    // The room is a flat arena. These must match the boss room width in Level.cpp.
    static constexpr float arena_left = 0.f;
    static constexpr float arena_right = 1920.f;

    // 0 - Dragon scales
    static constexpr float scales_windup_time = 0.7f;
    static constexpr int scale_count = 12;           // spread evenly all the way around
    static constexpr float scale_speed = 280.f;
    static constexpr float scale_size = 16.f;
    static constexpr float scale_life = 2.5f;

    // 1 - Draconic strike
    static constexpr float strike_windup_time = 0.55f;  // warning before the dash
    static constexpr float strike_dash_time = 0.4f;
    static constexpr float strike_speed = 1100.f;
    static constexpr float strike_reach = 70.f;         // the slash reaches this far in front of the body

    // 2 - Fireball
    static constexpr float fireball_windup_time = 0.6f;
    static constexpr float fireball_speed = 380.f;
    static constexpr float fireball_size = 30.f;

    // 3 - Planted blade
    static constexpr float blade_windup_time = 0.5f;
    static constexpr float blade_planted_time = 1.4f;   // the blade waits this long, then explodes
    static constexpr float blade_explode_time = 0.3f;
    static constexpr float explosion_width = 360.f;
    static constexpr float explosion_height = 260.f;

    // 4 - Burning ground
    static constexpr float fire_windup_time = 0.9f;     // the floor glows first (warning)
    static constexpr float fire_burn_time = 2.f;        // how long the floor burns
    static constexpr float fire_height = 40.f;          // flames this high hurt, so jump higher than this

    void chooseAttack();
    void clampToArena();
    void throwScales(EnemyWorld& world);
    void throwFireball(sf::Vector2f player_center, EnemyWorld& world);

public:
    DraconicLord(float x, float y);

    void update(float delta_time, const std::vector<sf::RectangleShape>& solids,
        sf::Vector2f player_center, EnemyWorld& world) override;
    void draw(sf::RenderWindow& window) override;
    bool isBoss() const override { return true; }
    bool hurtsOnContact() const override { return false; }
    bool getAttackHitbox(sf::FloatRect& box) const override;
};
