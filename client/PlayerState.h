#pragma once

#include <cmath>
#include <cstdint>

#include "config/ClientConfig.h"

static constexpr float MOVE_DURATION = 0.25f;

enum class Direction : uint8_t { NORTH = 0, SOUTH = 1, EAST = 2, WEST = 3, NONE = 4 };

// Posición y dirección del propio jugador en el cliente, con interpolación
// entre tile anterior y actual para que el movimiento se vea continuo
// (ver move_progress/pixel_x/pixel_y) en vez de saltar de tile a tile.
struct PlayerState {
    int tile_x = 50;
    int tile_y = 50;
    int prev_tile_x = 50;
    int prev_tile_y = 50;
    float move_progress = 1.0f;
    Direction direction = Direction::SOUTH;

    bool is_moving() const { return move_progress < 1.0f; }

    float pixel_x() const {
        int tile_size_px = ClientConfig::instance().tile_size();
        float from = static_cast<float>(prev_tile_x * tile_size_px);
        float to = static_cast<float>(tile_x * tile_size_px);
        return from + (to - from) * move_progress;
    }

    float pixel_y() const {
        int tile_size_px = ClientConfig::instance().tile_size();
        float from = static_cast<float>(prev_tile_y * tile_size_px);
        float to = static_cast<float>(tile_y * tile_size_px);
        return from + (to - from) * move_progress;
    }

    // Antes funciones globales en WorldState.h/.cpp (dist_to_player_tiles /
    // manhattan_dist_to_player_tiles); operan únicamente sobre tile_x/tile_y
    // de este PlayerState, así que pasan a ser métodos suyos.
    float dist_to_player_tiles(uint16_t x, uint16_t y) const {
        float dx = static_cast<float>(x) - static_cast<float>(tile_x);
        float dy = static_cast<float>(y) - static_cast<float>(tile_y);
        return std::sqrt(dx * dx + dy * dy);
    }

    // Distancia Manhattan usada para validar rango de hechizos y armas a
    // distancia ANTES de spawnear el efecto visual/sonido. La euclidiana de
    // dist_to_player_tiles da una distancia menor en diagonal, lo que hacía
    // que el cliente creyera estar en rango cuando el servidor ya lo rechazaba.
    int manhattan_dist_to_player_tiles(uint16_t x, uint16_t y) const {
        int dx = static_cast<int>(x) - static_cast<int>(tile_x);
        int dy = static_cast<int>(y) - static_cast<int>(tile_y);
        return std::abs(dx) + std::abs(dy);
    }

    void move_to(int new_tile_x, int new_tile_y, Direction dir) {
        prev_tile_x = tile_x;
        prev_tile_y = tile_y;
        tile_x = new_tile_x;
        tile_y = new_tile_y;
        direction = dir;
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
