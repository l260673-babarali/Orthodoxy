#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// The knight: movement, combat, health and animation
class Player
{
private:
    // Hitbox (invisible unless debug_hitbox is true)
    sf::RectangleShape shape;
    bool debug_hitbox = false;

    static constexpr float hitbox_width = 50.f;
    static constexpr float stand_height = 80.f;
    static constexpr float crouch_height = 44.f;

    float velocity_y = 0.f;     // up/down speed (negative = going up)
    bool is_on_ground = false;

    // Crouch
    bool is_crouching = false;
    static constexpr float crouch_speed_multiplier = 0.45f; // crouch walk is slower
    bool canStandUp(const std::vector<sf::RectangleShape>& solids) const;

    // Dash
    bool can_dash = true;           // one dash per jump; refilled when we touch the ground
    bool is_dashing = false;
    float dash_speed = 650.f;
    float dash_duration = 0.2f;
    float dash_timer = 0.f;
    float dash_cooldown = 0.4f;
    float dash_cooldown_timer = 0.f;

    // Strike
    bool  is_striking = false;
    bool  strike_was_held = false;       // was the strike key down last frame? (to detect a fresh press)
    float strike_duration = 0.25f;      // whole animation length
    float strike_active_start = 0.06f;   // hitbox turns on
    float strike_active_end = 0.18f;     // hitbox turns off
    float strike_timer = 0.f;
    float strike_cooldown = 0.35f;
    float strike_cooldown_timer = 0.f;
    float strike_range = 60.f;          // how far the hit reaches in front
    float strike_height = 50.f;
    int   strike_id = 0;                // new number every swing (so an enemy is hit once per swing)

    // Health
    int   max_health = 5;
    int   health = 5;
    float invincible_time = 1.0f;       // how long we are safe after being hit
    float invincible_timer = 0.f;
    float knockback_timer = 0.f;
    float knockback_push_x = 0.f;

    // Coyote time + double jump
    float coyote_time = 0.1f;           // can still jump this long after walking off a ledge
    float coyote_timer = 0.f;
    int max_air_jumps = 1;
    int air_jumps_left = 1;
    bool jump_was_held = false;

    float facing_direction = 1.f;       // 1 = right, -1 = left

    // Wall climb
    bool is_clinging = false;
    float wall_dir = 0.f;               // which side the wall is on (-1 left, 1 right)
    static constexpr float wall_slide_speed = 70.f;
    static constexpr float climb_velocity = -400.f;
    static constexpr float min_wall_height = 300.f;   // only tall solids count as climbable walls
    static constexpr float input_buffer_time = 0.08f;
    float jump_buffer_timer = 0.f;
    float strike_buffer_timer = 0.f;

    float detectWall(const std::vector<sf::RectangleShape>& solids) const;

    // Graphics
    sf::Texture sheet_texture;   // must come BEFORE the sprite
    sf::Sprite sprite;
    float anim_timer = 0.f;
    float sprite_scale = 2.0f;

public:
    Player();

    void update(
        float delta_time,
        const std::vector<sf::RectangleShape>& solids,
        float world_width
    );

    void draw(sf::RenderWindow& window);
    sf::Vector2f getCenter() const;

    bool isStrikeActive() const;
    sf::FloatRect getStrikeHitbox() const;
    int getStrikeId() const;

    void takeDamage(int amount, float from_direction);   // from_direction: -1 push left, 1 push right
    sf::FloatRect getBounds() const;
    float getFacing() const;
    int getHealth() const;
    int getMaxHealth() const;

    void reset();   // put the player back at the start with full health
};
