#pragma once

#include <cstdint>

// Layout del body (igual para todas las razas)
// Orden de direcciones: SUR=0, NORTE=1, OESTE=2, ESTE=3
struct BodyLayout {
    static constexpr int FRAME_W  = 27;
    static constexpr int DIR_Y[4] = {1,  45, 92,  138};
    static constexpr int DIR_H[4] = {43, 46, 45,  46};
    static constexpr int DIR_N[4] = {6,  6,  5,   5};
};

// Layout de cabeza por sprite_id (raza)
struct HeadLayout {
    int width;
    int dir_y[4];
    int dir_h[4];
    int overlaps[4];  // SUR, NORTE, OESTE, ESTE

    static HeadLayout for_sprite(uint8_t sprite_id) {
        switch (sprite_id) {
            case 1: return {34, {0, 60, 120, 180}, {60, 60, 60, 60}, {38, 40, 38, 38}};  // humano
            case 2: return {30, {0, 60, 120, 180}, {60, 60, 60, 60}, {34, 38, 38, 42}};  // elfo
            case 3: return {30, {0, 60, 120, 180}, {60, 60, 60, 60}, {46, 44, 40, 38}};  // enano
            case 4: return {30, {0, 60, 120, 180}, {60, 60, 60, 60}, {38, 36, 34, 32}};  // gnomo
            default: return {34, {0, 60, 120, 180}, {60, 60, 60, 60}, {38, 40, 38, 38}};
        }
    }
};