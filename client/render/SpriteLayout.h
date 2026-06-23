#pragma once

#include <cstdint>

// Layout del body (igual para todas las razas)
// Orden de direcciones: SUR=0, NORTE=1, OESTE=2, ESTE=3
struct BodyLayout {
    static constexpr int FRAME_W = 27;
    static constexpr int DIR_Y[4] = {1, 45, 93, 139};
    static constexpr int DIR_H[4] = {44, 47, 46, 47};
    static constexpr int DIR_N[4] = {6, 6, 5, 5};
};

// Layout de cabeza por sprite_id (raza)
struct HeadLayout {
    int width;
    int dir_y[4];
    int dir_h[4];
    int overlaps[4];
    int offset_x[4];
    float armor_scale_y = 1.0f;
    // No se aplica tabla: son las 4 razas jugables fijas del juego, cada
    // una con un literal de layout que no es un valor único sino un grupo
    // de 5 campos relacionados — ya está expresado como tabla de hecho,
    // solo que en forma de switch en vez de std::array/map.
    static HeadLayout for_sprite(uint8_t sprite_id) {
        switch (sprite_id) {
            case 1:
                return {15,  {0, 21, 42, 63}, {21, 21, 21, 21}, {11, 14, 11, 11}, {0, 0, 0, 0},
                        1.0f};  // humano
            case 2:
                return {19,  {0, 21, 42, 63}, {21, 21, 21, 21}, {11, 14, 14, 14}, {0, 0, 0, 0},
                        1.0f};  // elfo
            case 3:
                return {17,   {0, 20, 40, 60}, {20, 20, 20, 20}, {21, 23, 24, 24}, {0, 0, 0, 0},
                        0.75f};  // enano
            case 4:
                return {17,  {0, 25, 50, 75}, {25, 25, 25, 25}, {16, 18, 24, 23}, {0, -1, 0, 0},
                        1.0f};  // gnomo
            default:
                return {15,  {0, 21, 42, 63}, {21, 21, 21, 21}, {11, 14, 11, 11}, {0, 0, 0, 0},
                        1.0f};
        }
    }
};
