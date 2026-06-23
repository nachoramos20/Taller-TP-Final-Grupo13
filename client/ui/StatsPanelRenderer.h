#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2pp/SDL2pp.hh>

#include "../config/UiConstants.h"

#include "SpellSystem.h"

// Regiones clickeables que el último render() calculó; StatsPanel las
// guarda para resolver handle_event() en el frame siguiente (mismo
// patrón que InventoryLayout/InventoryRenderer).
struct StatsLayout {
    static constexpr int MAX_SPELL_BTNS = 3;
    SDL_Rect inv_btn_rect{};
    SDL_Rect help_btn_rect{};
    SDL_Rect spell_btn_rect[MAX_SPELL_BTNS]{};
    uint8_t spell_btn_id[MAX_SPELL_BTNS]{0, 0, 0};
    int spell_btn_count = 0;
};

// Qué dibujar: copia liviana de los datos de StatsPanel y SpellSystem
// que el renderer necesita para ese frame.
struct StatsRenderInput {
    const std::string* username;
    uint16_t hp, max_hp, mp, max_mp;
    uint32_t gold, exp;
    uint8_t level;
    bool meditating, is_ghost;
    uint8_t cls, eq_weapon_item;
    bool help_visible;
    bool cast_mode;
    uint8_t selected_spell;
    const std::vector<SpellInfo>* spells;
};

// Todo el dibujado del panel de stats (barras de HP/MP/EXP, oro, botones
// de inventario/ayuda, botones de hechizo, popup de ayuda con la lista de
// comandos/atajos/cheats). Extraída de StatsPanel, que mezclaba esto con
// el manejo de eventos de mouse y el estado de selección de hechizo
// (ver SpellSystem).
//
// Queda ~350 líneas de .cpp, por encima de las 200: la mitad es
// render_help_overlay(), un volcado de texto estático (comandos/atajos/
// cheats) ya aislado en su propio método. No se fragmenta más porque
// sigue siendo una sola responsabilidad ("dibujar el panel y su ayuda"),
// sin parseo/cálculo/persistencia mezclado — el motivo real para
// dividir, no el recuento de líneas en sí (mismo criterio que se aplicó
// en InventoryRenderer).
class StatsPanelRenderer {
public:
    StatsPanelRenderer(SDL2pp::Renderer& renderer, TTF_Font* font, int font_size);

    StatsLayout render(int screen_w, int screen_h, const StatsRenderInput& input);

    static constexpr int PANEL_W = UI_STATS_PANEL_WIDTH;

private:
    void draw_text(const std::string& text, int x, int y, SDL_Color color);
    void draw_text_right(const std::string& text, int right_x, int y, SDL_Color color);
    void draw_bar(int x, int y, int w, int h, float fraction, SDL_Color fill, SDL_Color bg);
    void draw_rounded_rect(int x, int y, int w, int h, SDL_Color color);

    void render_help_overlay(int screen_w, int screen_h);

    SDL2pp::Renderer& _renderer;
    TTF_Font* _font;
    int _font_size;
};
