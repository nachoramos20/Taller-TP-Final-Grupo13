#pragma once

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2pp/SDL2pp.hh>

// Muestra la posición (tile_x, tile_y) del jugador en pantalla; se puede
// ocultar/mostrar con un atajo de teclado.
class PositionLabel {
public:
    PositionLabel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size = 14);
    ~PositionLabel();

    void toggle_visibility();
    void update(int tile_x, int tile_y);
    void render(int screen_w, int screen_h);

private:
    bool _visible = false;

    void draw_text(const std::string& text, int x, int y, SDL_Color color);

    SDL2pp::Renderer& _renderer;
    TTF_Font* _font = nullptr;
    int _font_size;

    int _tile_x = 0;
    int _tile_y = 0;
};
