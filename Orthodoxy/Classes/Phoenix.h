#pragma once
#include "Enemy.h"

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
