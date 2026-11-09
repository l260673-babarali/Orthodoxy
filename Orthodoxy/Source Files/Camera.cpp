#include "../Classes/Camera.h"

Camera::Camera()
{
    view.setSize({ 800.f, 600.f });
    view.setCenter({ 400.f, 300.f });
}

void Camera::follow(sf::Vector2f target, float worldWidth, float worldTop)
{
    float halfViewWidth = view.getSize().x / 2.f;
    float halfViewHeight = view.getSize().y / 2.f;

    float cameraX = target.x;
    if (cameraX < halfViewWidth)
        cameraX = halfViewWidth;
    if (cameraX > worldWidth - halfViewWidth)
        cameraX = worldWidth - halfViewWidth;

    float cameraY = target.y;
    if (cameraY > 300.f)
        cameraY = 300.f;
    if (cameraY < worldTop + halfViewHeight)
        cameraY = worldTop + halfViewHeight;

    view.setCenter({ cameraX, cameraY });
}

void Camera::apply(sf::RenderWindow& window)
{
    window.setView(view);
}

sf::Vector2f Camera::getCenter() const
{
    return view.getCenter();
}