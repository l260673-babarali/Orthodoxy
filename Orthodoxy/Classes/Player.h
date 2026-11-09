#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Player
{
private:
    enum class AnimState { Idle, Walk, CrouchIdle, CrouchWalk, Air, Dash };

    // Hitbox (invisible unless debugHitbox is true)
    sf::RectangleShape shape;
    bool debugHitbox = false;

    static constexpr float hitboxWidth = 50.f;
    static constexpr float standHeight = 80.f;
    static constexpr float crouchHeight = 44.f;

    float velocityY = 0.f;
    bool OnGround = false;

    // Crouch
    bool isCrouching = false;
    static constexpr float crouchSpeedMultiplier = 0.45f; // crouch walk is slower
    bool canStandUp(const std::vector<sf::RectangleShape>& solids) const;

    // Dash
    bool canDash = true;
    bool isDashing = false;
    float dashSpeed = 650.f;
    float dashDuration = 0.2f;
    float dashTimer = 0.f;
    float dashCooldown = 0.4f;
    float dashCooldownTimer = 0.f;

    // Strike
    bool  isStriking = false;
    bool  strikeWasHeld = false;
    float strikeDuration = 0.25f;      // whole animation length
    float strikeActiveStart = 0.06f;   // hitbox turns on
    float strikeActiveEnd = 0.18f;     // hitbox turns off
    float strikeTimer = 0.f;
    float strikeCooldown = 0.35f;
    float strikeCooldownTimer = 0.f;
    float strikeRange = 60.f;          // how far the hit reaches in front
    float strikeHeight = 50.f;
    int   strikeId = 0;                // new number every swing (so an enemy is hit once per swing)

    // Health
    int   maxHealth = 5;
    int   health = 5;
    float invincibleTime = 1.0f;
    float invincibleTimer = 0.f;
    float knockbackTimer = 0.f;
    float knockbackVX = 0.f;

    // Coyote time + double jump
    float coyoteTime = 0.1f;
    float coyoteTimer = 0.f;
    int maxAirJumps = 1;
    int airJumpsLeft = 1;
    bool jumpWasHeld = false;

    float facingDirection = 1.f;

    // Wall climb
    bool isClinging = false;
    float wallDir = 0.f;
    static constexpr float wallSlideSpeed = 70.f;
    static constexpr float climbVelocity = -400.f;
    static constexpr float minWallHeight = 300.f;
    static constexpr float inputBufferTime = 0.08f;
    float jumpBufferTimer = 0.f;
    float strikeBufferTimer = 0.f;

    float detectWall(const std::vector<sf::RectangleShape>& solids) const;

    // Graphics 
    sf::Texture sheetTexture;   // must come BEFORE the sprite
    sf::Sprite sprite;
    float animTimer = 0.f;
    float spriteScale = 2.0f;
    bool spriteFacesRight = true;


public:
    Player();

    void update(
        float deltaTime,
        const std::vector<sf::RectangleShape>& solids,
        float worldWidth
    );

    void draw(sf::RenderWindow& window);
    sf::Vector2f getCenter() const;

    bool isStrikeActive() const;
    sf::FloatRect getStrikeHitbox() const;
    int getStrikeId() const;

    void takeDamage(int amount, float fromDirection);   // fromDirection: -1 push left, 1 push right
    sf::FloatRect getBounds() const;
    float getFacing() const;
    int getHealth() const;
    int getMaxHealth() const;
};