#include "../Classes/Camera.h"

Camera::Camera()
{
    // 960 x 540 camera ka visible world area hai; 1280 x 720 window ki 16:9 ratio same rehti hai.
    view.setSize({ 960.f, 540.f });
    view.setCenter({ 480.f, 270.f });
}

void Camera::setViewHeight(float height)
{
    view.setSize({ height * 16.f / 9.f, height });
}

// Center the view on the target, but clamp it so we never see outside the level
void Camera::follow(sf::Vector2f target, float world_width, float world_top, float world_bottom)
{
    // Camera ka half width nikalte hain: width / 2. Edge par camera ko half-screen margin chahiye.
    float half_view_width = view.getSize().x / 2.f;
    // Camera ki half height = visible height / 2; isi se top/bottom boundary check hota hai.
    float half_view_height = view.getSize().y / 2.f;

    // Left and right edges
    float camera_x = target.x;
    if (camera_x < half_view_width)
        camera_x = half_view_width;
    // Right limit = world ki total width - camera ki half width; is se world ke bahar ka area nazar nahi aata.
    if (camera_x > world_width - half_view_width)
        camera_x = world_width - half_view_width;
    // A level narrower than the view (the boss room): keep it centred
    if (world_width <= view.getSize().x)
        camera_x = world_width / 2.f;

    // Top and bottom edges
    float camera_y = target.y;
    // Bottom limit = world ka last Y - half camera height; camera neeche extra space nahi dikhata.
    if (camera_y > world_bottom - half_view_height)
        camera_y = world_bottom - half_view_height;
    // Top limit = world ka first Y + half camera height; camera world se ooper nahi jata.
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


const sf::View& Camera::getView() const
{
    return view;
}