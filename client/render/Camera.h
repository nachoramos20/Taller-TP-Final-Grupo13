#pragma once

#include "../game/PlayerState.h"

class Camera {
public:
    Camera(int screen_w, int screen_h);

    void follow(const PlayerState& player);
    int world_to_screen_x(float world_x) const;
    int world_to_screen_y(float world_y) const;
    int tile_to_screen_x(int tile_x) const;
    int tile_to_screen_y(int tile_y) const;
    void set_screen_size(int w, int h);

private:
    int   _screen_w, _screen_h;
    float _x, _y;
};