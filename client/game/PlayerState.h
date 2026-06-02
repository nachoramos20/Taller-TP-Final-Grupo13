#pragma once

#include <cstdint>

static constexpr int TILE_SIZE = 64;
static constexpr float MOVE_DURATION = 0.25f;

enum class Direction : uint8_t {
    NORTH = 0,
    SOUTH = 1,
    EAST  = 2,
    WEST  = 3,
    NONE  = 4
};

struct PlayerState {
    int tile_x = 50;
    int tile_y = 50;
    int prev_tile_x = 50;
    int prev_tile_y = 50;
    float move_progress = 1.0f;
    Direction direction = Direction::SOUTH;

    bool is_moving() const { return move_progress < 1.0f; }

    float pixel_x() const {
        float from = static_cast<float>(prev_tile_x * TILE_SIZE);
        float to   = static_cast<float>(tile_x * TILE_SIZE);
        return from + (to - from) * move_progress;
    }

    float pixel_y() const {
        float from = static_cast<float>(prev_tile_y * TILE_SIZE);
        float to   = static_cast<float>(tile_y * TILE_SIZE);
        return from + (to - from) * move_progress;
    }

    void move_to(int new_tile_x, int new_tile_y, Direction dir) {
        prev_tile_x   = tile_x;
        prev_tile_y   = tile_y;
        tile_x        = new_tile_x;
        tile_y        = new_tile_y;
        direction     = dir;
        move_progress = 0.0f;
    }

    void update(float dt) {
        if (is_moving()) {
            move_progress += dt / MOVE_DURATION;
            if (move_progress >= 1.0f)
                move_progress = 1.0f;
        }
    }
};