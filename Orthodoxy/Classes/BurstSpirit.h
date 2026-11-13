#pragma once
#include "Enemy.h"

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
