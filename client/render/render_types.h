#pragma once

#include <string>

// Tipos de render compartidos entre AnimationSystem (que los produce/consume)
// y WorldRenderer (que los usa para posicionar overlays y pasar el equipo a
// dibujar). Antes vivían dentro de AnimationSystem.h y WorldRenderer los
// obtenía por inclusión transitiva.

// Límites reales del sprite ya dibujado en pantalla
struct SpriteBounds {
    int top_y;
    int center_x;
    int width;
};

// Equipo visual a renderizar encima del personaje
struct EquipVisual {
    std::string armor_path;
    std::string helmet_path;
    int         helmet_src_x = 0;
    int         helmet_src_y = 0;
    int         helmet_src_w = 0;
    int         helmet_src_h = 0;
    int         helmet_offset_y[4] = {0, 0, 0, 0};
    int         helmet_offset_x[4] = {0, 0, 0, 0};
    std::string weapon_path;
    std::string shield_path;
};
