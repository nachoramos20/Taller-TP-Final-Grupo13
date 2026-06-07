#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cstdint>

// Panel lateral derecho: vida, maná, oro, nivel, botón inventario.
// Devuelve true en open_inventory() si el botón fue presionado en el último frame.
class StatsPanel {
public:
    StatsPanel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size = 14);
    ~StatsPanel();

    void update(uint16_t hp, uint16_t max_hp, uint16_t mp, uint16_t max_mp,
                uint32_t gold, uint8_t level, bool meditating, bool is_ghost);

    bool handle_event(const SDL_Event& e);

    void render(int screen_w, int screen_h);

    bool inventory_button_clicked() const { return _inv_clicked; }

    static constexpr int PANEL_W = 200;

private:
    void draw_text(const std::string& text, int x, int y, SDL_Color color);
    void draw_bar(int x, int y, int w, int h,
                  float fraction, SDL_Color fill, SDL_Color bg);
    void draw_rounded_rect(int x, int y, int w, int h, SDL_Color color);

    SDL2pp::Renderer& _renderer;
    TTF_Font*         _font      = nullptr;
    int               _font_size;

    uint16_t _hp     = 0,  _max_hp  = 1;
    uint16_t _mp     = 0,  _max_mp  = 1;
    uint32_t _gold   = 0;
    uint8_t  _level  = 1;
    bool     _meditating = false;
    bool     _is_ghost   = false;

    SDL_Rect _inv_btn_rect {};
    bool     _inv_clicked  = false;
};
