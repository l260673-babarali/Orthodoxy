#include "../Classes/Camera.h"

Camera::Camera()
{
    view.setSize({ 800.f, 600.f });
    view.setCenter({ 400.f, 300.f });
}

// Center the view on the target, but clamp it so we never see outside the level
void Camera::follow(sf::Vector2f target, float world_width, float world_top, float world_bottom)
{
    float half_view_width = view.getSize().x / 2.f;
    float half_view_height = view.getSize().y / 2.f;

    // Left and right edges
    float camera_x = target.x;
    if (camera_x < half_view_width)
        camera_x = half_view_width;
    if (camera_x > world_width - half_view_width)
        camera_x = world_width - half_view_width;

    // Top and bottom edges
    float camera_y = target.y;
    if (camera_y > world_bottom - half_view_height)
        camera_y = world_bottom - half_view_height;
    if (camera_y < world_top + half_view_height)
        camera_y = world_top + half_view_height;

    view.setCenter({ camera_x, camera_y });
}

// Make the window draw using this camera
void Camera::apply(sf::RenderWindow& window)
{
    window.setView(view);
}

sf::Vector2f Camera::getCenter() const
{
    return view.getCenter();
}
