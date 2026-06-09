#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class PositionLabel {
public:
    PositionLabel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size = 14);
    ~PositionLabel();

    void update(int tile_x, int tile_y);
    void render(int screen_w, int screen_h);

private:
    void draw_text(const std::string& text, int x, int y, SDL_Color color);

    SDL2pp::Renderer& _renderer;
    TTF_Font*         _font      = nullptr;
    int               _font_size;

    int _tile_x = 0;
    int _tile_y = 0;
};