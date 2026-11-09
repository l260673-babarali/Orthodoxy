
#include "Player.h"

Player::Player()
    : sprite(idleTexture)
{
    shape.setSize({ 50.f, 80.f });
    shape.setPosition({ 100.f, 100.f });
}

void Player::update(
    float deltaTime, const std::vector<sf::RectangleShape>& solids,
    float worldWidth
)
{
    const float speed = 200.f;
    const float gravity = 981.f;
    const float jumpVelocity = -550.f;
    const float doubleJumpVelocity = -480.f;

    if (invincibleTimer > 0.f)     invincibleTimer -= deltaTime;
    if (dashCooldownTimer > 0.f)   dashCooldownTimer -= deltaTime;
    if (strikeCooldownTimer > 0.f) strikeCooldownTimer -= deltaTime;

    if (isStriking)
    {
        strikeTimer += deltaTime;
        if (strikeTimer >= strikeDuration)
            isStriking = false;
    }

    // ---- Raw input (edge-detected) ----
    bool strikeHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J);
    bool strikePressed = strikeHeld && !strikeWasHeld;
    strikeWasHeld = strikeHeld;

    bool jumpHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    bool jumpPressed = jumpHeld && !jumpWasHeld;
    jumpWasHeld = jumpHeld;

    float inputDir = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) inputDir -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) inputDir += 1.f;

    // Input buffers: lets jump + strike count as "together" if pressed within a few ms
    if (jumpBufferTimer > 0.f)   jumpBufferTimer -= deltaTime;
    if (strikeBufferTimer > 0.f) strikeBufferTimer -= deltaTime;
    if (jumpPressed)   jumpBufferTimer = inputBufferTime;
    if (strikePressed) strikeBufferTimer = inputBufferTime;

    // ---- Wall detection / cling state ----
    float wallSide = detectWall(solids);
    bool touchingWall = (wallSide != 0.f) && !OnGround;

    if (isClinging)
    {
        bool movingAway = (inputDir != 0.f && inputDir == -wallDir);
        if (!touchingWall || movingAway)
            isClinging = false;
    }

    bool climbPressed = touchingWall
        && (jumpPressed || strikePressed)
        && jumpBufferTimer > 0.f && strikeBufferTimer > 0.f
        && inputDir != -wallSide
        && !isDashing;

    if (climbPressed)
    {
        isClinging = true;
        wallDir = wallSide;
        facingDirection = wallSide;
        velocityY = climbVelocity;
        isStriking = false;
        jumpBufferTimer = 0.f;
        strikeBufferTimer = 0.f;
    }

    // Normal jump/strike are disabled while at the wall, so they don't fire on the way to a climb
    bool nearWall = touchingWall || isClinging;

    // ---- Strike ----
    if (strikePressed && !isStriking && !isDashing && !nearWall && strikeCooldownTimer <= 0.f)
    {
        isStriking = true;
        strikeTimer = 0.f;
        strikeCooldownTimer = strikeCooldown;
        strikeId++;
    }

    // ---- Crouch ----
    bool crouchHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    bool wantCrouch = crouchHeld && OnGround && !isClinging;

    if (wantCrouch && !isCrouching)
    {
        // Shrink the box, but keep the feet in the same spot
        float bottom = shape.getPosition().y + shape.getSize().y;
        shape.setSize({ hitboxWidth, crouchHeight });
        shape.setPosition({ shape.getPosition().x, bottom - crouchHeight });
        isCrouching = true;
    }
    else if (!wantCrouch && isCrouching)
    {
        // Only stand up if there is room above our head
        if (canStandUp(solids))
        {
            float bottom = shape.getPosition().y + shape.getSize().y;
            shape.setSize({ hitboxWidth, standHeight });
            shape.setPosition({ shape.getPosition().x, bottom - standHeight });
            isCrouching = false;
        }
    }

    // ---- Horizontal input ----
    float moveX = 0.f;
    if (inputDir < 0.f)
    {
        moveX -= speed * deltaTime;
        if (!isClinging) facingDirection = -1.f;
    }
    if (inputDir > 0.f)
    {
        moveX += speed * deltaTime;
        if (!isClinging) facingDirection = 1.f;
    }

    if (isStriking && OnGround)
        moveX *= 0.3f;

    if (isCrouching)
        moveX *= crouchSpeedMultiplier;

    if (knockbackTimer > 0.f)
    {
        moveX += knockbackVX * deltaTime;
        knockbackTimer -= deltaTime;
    }

    // ---- Jump ----
    if (OnGround)
    {
        coyoteTimer = coyoteTime;
        airJumpsLeft = maxAirJumps;
    }
    else
    {
        coyoteTimer -= deltaTime;
    }

    if (jumpPressed && !isDashing && !nearWall)
    {
        if (coyoteTimer > 0.f)
        {
            velocityY = jumpVelocity;
            coyoteTimer = 0.f;
            OnGround = false;
        }
        else if (airJumpsLeft > 0)
        {
            velocityY = doubleJumpVelocity;
            airJumpsLeft--;
        }
    }

    // ---- Dash ----
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        && canDash && !isDashing && !isStriking && !isClinging && dashCooldownTimer <= 0.f)
    {
        isDashing = true;
        canDash = false;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
        velocityY = 0.f;
    }

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
        // Clinging: gravity still pulls us down, but only as a slow slide
        if (isClinging && velocityY > wallSlideSpeed)
            velocityY = wallSlideSpeed;
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
                shape.setPosition({ p.position.x, s.position.y - p.size.y });
                velocityY = 0.f;
            }
            else if (velocityY < 0.f)
            {
                shape.setPosition({ p.position.x, s.position.y + s.size.y });
                velocityY = 0.f;
            }
        }
    }

    // ---- Ground check ----
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

    if (OnGround)
        isClinging = false;

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
        isClinging = false;
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
    isClinging = false;
}

sf::FloatRect Player::getBounds() const { return shape.getGlobalBounds(); }
float Player::getFacing() const { return facingDirection; }
int Player::getHealth() const { return health; }
int Player::getMaxHealth() const { return maxHealth; }

float Player::detectWall(const std::vector<sf::RectangleShape>& solids) const
{
    sf::FloatRect b = shape.getGlobalBounds();
    sf::FloatRect leftProbe({ b.position.x - 2.f, b.position.y }, { 2.f, b.size.y });
    sf::FloatRect rightProbe({ b.position.x + b.size.x, b.position.y }, { 2.f, b.size.y });

    for (const auto& solid : solids)
    {
        sf::FloatRect s = solid.getGlobalBounds();
        if (s.size.y < minWallHeight)
            continue;
        if (leftProbe.findIntersection(s))  return -1.f;
        if (rightProbe.findIntersection(s)) return 1.f;
    }
    return 0.f;
}

bool Player::canStandUp(const std::vector<sf::RectangleShape>& solids) const
{
    // Imagine the tall standing box with feet in the same place
    float bottom = shape.getPosition().y + shape.getSize().y;
    sf::FloatRect standingBox(
        { shape.getPosition().x, bottom - standHeight },
        { hitboxWidth, standHeight - 1.f }
    );

    for (const auto& solid : solids)
    {
        if (standingBox.findIntersection(solid.getGlobalBounds()))
            return false; // something is above our head
    }
    return true;
}