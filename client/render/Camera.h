#pragma once

#include "../game/PlayerState.h"

class Camera {
public:
    Camera(int screen_w, int screen_h)
        : _screen_w(screen_w), _screen_h(screen_h),
          _x(0.0f), _y(0.0f) {}

    void follow(const PlayerState& player) {
        _x = player.pixel_x() - _screen_w / 2.0f;
        _y = player.pixel_y() - _screen_h / 2.0f;
    }

    int world_to_screen_x(float world_x) const {
        return static_cast<int>(world_x - _x);
    }

    int world_to_screen_y(float world_y) const {
        return static_cast<int>(world_y - _y);
    }

    int tile_to_screen_x(int tile_x) const {
        return world_to_screen_x(static_cast<float>(tile_x * TILE_SIZE));
    }

    int tile_to_screen_y(int tile_y) const {
        return world_to_screen_y(static_cast<float>(tile_y * TILE_SIZE));
    }

    void set_screen_size(int w, int h) {
        _screen_w = w;
        _screen_h = h;
    }

private:
    int   _screen_w, _screen_h;
    float _x, _y;
};