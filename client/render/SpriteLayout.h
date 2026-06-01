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

    static HeadLayout for_sprite(uint8_t sprite_id) {
        switch (sprite_id) {
            case 1: return {42, {12, 76, 140, 204}, {14, 14, 14, 14}};  // humano
            case 2: return {35, {14, 78, 142, 206}, {15, 15, 15, 15}};  // elfo
            case 3: return {40, {24, 88, 152, 216}, {14, 14, 14, 14}};  // enano
            case 4: return {37, {17, 81, 145, 209}, {21, 14, 21, 21}};  // gnomo
            default: return {42, {12, 76, 140, 204}, {14, 14, 14, 14}};
        }
    }
};