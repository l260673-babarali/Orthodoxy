#pragma once
#include "Enemy.h"

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
