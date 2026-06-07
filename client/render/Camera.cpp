#include "Camera.h"

Camera::Camera(int screen_w, int screen_h, int panel_w)
    : _screen_w(screen_w), _screen_h(screen_h),
      _panel_w(panel_w),
      _x(0.0f), _y(0.0f) {}

void Camera::follow(const PlayerState& player) {
    // Centrar al jugador en la zona de juego (excluye el panel lateral)
    int area_w = _screen_w - _panel_w;
    _x = player.pixel_x() - area_w / 2.0f;
    _y = player.pixel_y() - _screen_h / 2.0f;
}

int Camera::world_to_screen_x(float world_x) const {
    return static_cast<int>(world_x - _x);
}

int Camera::world_to_screen_y(float world_y) const {
    return static_cast<int>(world_y - _y);
}

int Camera::tile_to_screen_x(int tile_x) const {
    return world_to_screen_x(static_cast<float>(tile_x * TILE_SIZE));
}

int Camera::tile_to_screen_y(int tile_y) const {
    return world_to_screen_y(static_cast<float>(tile_y * TILE_SIZE));
}

void Camera::set_screen_size(int w, int h, int panel_w) {
    _screen_w = w;
    _screen_h = h;
    if (panel_w >= 0) _panel_w = panel_w;
}
