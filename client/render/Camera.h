#pragma once

#include "../PlayerState.h"

// Traduce coordenadas de mundo/tile a coordenadas de pantalla, siguiendo
// al jugador y dejando espacio para el panel lateral de stats.
class Camera {
public:
    Camera(int screen_w, int screen_h, int panel_w = 0);

    void follow(const PlayerState& player);
    int world_to_screen_x(float world_x) const;
    int world_to_screen_y(float world_y) const;
    int tile_to_screen_x(int tile_x) const;
    int tile_to_screen_y(int tile_y) const;

    void set_screen_size(int w, int h, int panel_w = -1);
    void set_tile_size(int tile_size);

    int game_area_w() const { return _screen_w - _panel_w; }
    int game_area_h() const { return _screen_h; }

private:
    int   _screen_w, _screen_h;
    int   _panel_w;   // ancho reservado por el panel lateral
    float _x, _y;
    int   _tile_size; // tamaño del tile (dinámico desde config)
};
