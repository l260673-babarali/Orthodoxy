#include "Camera.h"

Camera::Camera()
{
    view.setSize({ 800.f, 600.f });
    view.setCenter({ 400.f, 300.f });
}

void Camera::follow(sf::Vector2f target, float worldWidth)
{
    float halfViewWidth = view.getSize().x / 2.f;

    float cameraX = target.x;

    // Left boundary
    if (cameraX < halfViewWidth)
        cameraX = halfViewWidth;

    // Right boundary
    if (cameraX > worldWidth - halfViewWidth)
        cameraX = worldWidth - halfViewWidth;

    view.setCenter({ cameraX, 300.f });
}

void Camera::apply(sf::RenderWindow& window)
{
    window.setView(view);
}