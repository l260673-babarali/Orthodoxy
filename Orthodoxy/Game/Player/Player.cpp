
#include "Player.h"

Player::Player()
    : sprite(idleTexture)
{
    shape.setSize({ 50.f, 80.f });
    shape.setPosition({ 100.f, 100.f });
}

void Player::update(
    float deltaTime,
    const std::vector<sf::RectangleShape>& solids,
    float worldWidth
)
{
    const float speed = 200.f;
    const float gravity = 981.f;
    const float jumpVelocity = -550.f;
    const float doubleJumpVelocity = -480.f;

    if (invincibleTimer > 0.f)
        invincibleTimer -= deltaTime;

    // Tick dash cooldown
    if (dashCooldownTimer > 0.f)
        dashCooldownTimer -= deltaTime;

    // Tick strike cooldown
    if (strikeCooldownTimer > 0.f)
        strikeCooldownTimer -= deltaTime;

    // Tick the strike itself
    if (isStriking)
    {
        strikeTimer += deltaTime;
        if (strikeTimer >= strikeDuration)
            isStriking = false;
    }

    // Strike input (edge-detected so holding J doesn't spam)
    bool strikeHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J);
    bool strikePressed = strikeHeld && !strikeWasHeld;
    strikeWasHeld = strikeHeld;

    if (strikePressed && !isStriking && !isDashing && strikeCooldownTimer <= 0.f)
    {
        isStriking = true;
        strikeTimer = 0.f;
        strikeCooldownTimer = strikeCooldown;
        strikeId++;
    }

    // ---- Input ----
    float moveX = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        moveX -= speed * deltaTime;
        facingDirection = -1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        moveX += speed * deltaTime;
        facingDirection = 1.f;
    }
    if (isStriking && OnGround)
        moveX *= 0.3f;
    // Knockback after being hit
    if (knockbackTimer > 0.f)
    {
        moveX += knockbackVX * deltaTime;
        knockbackTimer -= deltaTime;
    }

    // ---- Jump input (edge-detected: holding Space won't trigger a double jump) ----
    bool jumpHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    bool jumpPressed = jumpHeld && !jumpWasHeld;
    jumpWasHeld = jumpHeld;

    // Coyote timer: refilled while grounded, drains once we leave the ground
    if (OnGround)
    {
        coyoteTimer = coyoteTime;
        airJumpsLeft = maxAirJumps;
    }
    else
    {
        coyoteTimer -= deltaTime;
    }

    if (jumpPressed && !isDashing)
    {
        if (coyoteTimer > 0.f)
        {
            // Normal jump (also works for a few frames after leaving a ledge)
            velocityY = jumpVelocity;
            coyoteTimer = 0.f;   // can't coyote-jump twice
            OnGround = false;
        }
        else if (airJumpsLeft > 0)
        {
            // Double jump
            velocityY = doubleJumpVelocity;
            airJumpsLeft--;
        }
    }

    // Start dash (ground or air)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        && canDash && !isDashing && !isStriking && dashCooldownTimer <= 0.f)
    {
        isDashing = true;
        canDash = false;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
        velocityY = 0.f;
    }

    // Dash overrides normal horizontal movement and suspends gravity
    if (isDashing)
    {
        moveX = facingDirection * dashSpeed * deltaTime;
        velocityY = 0.f;

        dashTimer -= deltaTime;
        if (dashTimer <= 0.f)
            isDashing = false;
    }
    else
    {
        velocityY += gravity * deltaTime;
    }

    // ---- Horizontal movement + collision ----
    shape.move({ moveX, 0.f });

    if (moveX != 0.f)
    {
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

                // Hitting a wall cancels the dash
                isDashing = false;
            }
        }
    }

    // ---- Vertical movement + collision ----
    shape.move({ 0.f, velocityY * deltaTime });

    for (const auto& solid : solids)
    {
        sf::FloatRect p = shape.getGlobalBounds();
        sf::FloatRect s = solid.getGlobalBounds();

        if (p.findIntersection(s))
        {
            if (velocityY > 0.f)
            {
                // Falling: land on top
                shape.setPosition({ p.position.x, s.position.y - p.size.y });
                velocityY = 0.f;
            }
            else if (velocityY < 0.f)
            {
                // Rising: bump head on the underside
                shape.setPosition({ p.position.x, s.position.y + s.size.y });
                velocityY = 0.f;
            }
        }
    }

    // ---- Ground check: is there a solid 1px below us? ----
    OnGround = false;
    if (velocityY >= 0.f)
    {
        sf::FloatRect probe(
            { shape.getPosition().x, shape.getPosition().y + 1.f },
            shape.getSize()
        );

        for (const auto& solid : solids)
        {
            if (probe.findIntersection(solid.getGlobalBounds()))
            {
                OnGround = true;
                break;
            }
        }
    }

    // Recharge dash when grounded and not mid-dash
    if (OnGround && !isDashing)
        canDash = true;

    // ---- World boundaries ----
    if (shape.getPosition().x < 0.f)
        shape.setPosition({ 0.f, shape.getPosition().y });

    if (shape.getPosition().x + shape.getSize().x > worldWidth)
        shape.setPosition({ worldWidth - shape.getSize().x, shape.getPosition().y });

    // Died: respawn with full health
    if (health <= 0)
    {
        shape.setPosition({ 100.f, 100.f });
        velocityY = 0.f;
        health = maxHealth;
        invincibleTimer = 1.5f;
    }
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(shape);
    bool blinkOff = invincibleTimer > 0.f
        && static_cast<int>(invincibleTimer * 10.f) % 2 == 0;
    if (!blinkOff)
        window.draw(shape);

    if (isStrikeActive())
    {
        sf::FloatRect hb = getStrikeHitbox();
        sf::RectangleShape debugStrike(hb.size);
        debugStrike.setPosition(hb.position);
        debugStrike.setFillColor(sf::Color(255, 0, 0, 120));
        window.draw(debugStrike);
    }
}

sf::Vector2f Player::getCenter() const
{
    return {
        shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f
    };
}

bool Player::isStrikeActive() const
{
    return isStriking
        && strikeTimer >= strikeActiveStart
        && strikeTimer <= strikeActiveEnd;
}

sf::FloatRect Player::getStrikeHitbox() const
{
    float x;
    if (facingDirection > 0.f)
        x = shape.getPosition().x + shape.getSize().x;   // right edge
    else
        x = shape.getPosition().x - strikeRange;         // left of player

    float y = shape.getPosition().y + shape.getSize().y * 0.25f;

    return sf::FloatRect({ x, y }, { strikeRange, strikeHeight });
}

int Player::getStrikeId() const
{
    return strikeId;
}

void Player::takeDamage(int amount, float fromDirection)
{
    if (invincibleTimer > 0.f)
        return;

    health -= amount;
    invincibleTimer = invincibleTime;

    knockbackTimer = 0.2f;
    knockbackVX = fromDirection * 300.f;
    velocityY = -250.f;
    OnGround = false;

    isDashing = false;
    isStriking = false;
}

sf::FloatRect Player::getBounds() const { return shape.getGlobalBounds(); }
float Player::getFacing() const { return facingDirection; }
int Player::getHealth() const { return health; }
int Player::getMaxHealth() const { return maxHealth; }