#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2pp/SDL2pp.hh>

#include "../config/UiConstants.h"

#include "SpellSystem.h"
#include "StatsPanelRenderer.h"

class GameAudioService;

// Dueño de los datos del jugador (hp/mp/oro/nivel/exp/clase) y de la
// interacción de mouse (botones de inventario/ayuda/hechizo). El dibujado
// vive en StatsPanelRenderer y la selección de hechizo en SpellSystem,
// ambas usadas por composición — ver render().
class StatsPanel {
public:
    StatsPanel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size = 14,
               GameAudioService* audio = nullptr);
    ~StatsPanel();

    void set_username(const std::string& username) { _username = username; }

    void update(uint16_t hp, uint16_t max_hp, uint16_t mp, uint16_t max_mp, uint32_t gold,
                uint8_t level, uint32_t exp, bool meditating, bool is_ghost, uint8_t cls,
                uint8_t equipped_weapon_item_id);

    bool handle_event(const SDL_Event& e);
    void render(int screen_w, int screen_h);

    bool inventory_button_clicked() const { return _inv_clicked; }

    // Modo hechizo: si está activo, los clicks sobre enemigos lanzan _selected_spell.
    bool cast_mode_active() const { return _spell_system.cast_mode_active(); }
    uint8_t selected_spell() const { return _spell_system.selected_spell(); }

    // Acceso al MP actual (para validación client-side antes de spawnear VFX)
    uint16_t current_mp() const { return _mp; }

    // Nivel propio (para replicar localmente el chequeo de fair-play antes
    // de spawnear VFX de ataque/hechizo contra otro jugador).
    uint8_t current_level() const { return _level; }

    // Costo de maná y rango del hechizo actualmente seleccionado (0 si ninguno)
    uint16_t selected_spell_mana_cost() const {
        return _spell_system.selected_spell_mana_cost(_cls);
    }
    int selected_spell_range() const { return _spell_system.selected_spell_range(_cls); }

    // Atajos de teclado: activa el hechizo por índice (0-based), o usa poción
    void activate_spell_by_index(int index);
    void use_potion_shortcut();  // retorna true si había una poción en inventario

    // Para que el shortcut de poción acceda al inventario
    void set_inventory_ref(const uint8_t* inv, int size);

    bool help_visible() const { return _help_visible; }
    void toggle_help() { _help_visible = !_help_visible; }
    void close_help() { _help_visible = false; }

    static constexpr int PANEL_W = UI_STATS_PANEL_WIDTH;

private:
    SDL2pp::Renderer& _renderer;
    GameAudioService* _audio = nullptr;
    TTF_Font* _font = nullptr;
    int _font_size;

    // Construido en el cuerpo del constructor (necesita _font ya
    // abierto), por eso es optional en vez de miembro directo.
    std::optional<StatsPanelRenderer> _render_impl;
    SpellSystem _spell_system;

    std::string _username;

    uint16_t _hp = 0, _max_hp = 1;
    uint16_t _mp = 0, _max_mp = 1;
    uint32_t _gold = 0;
    uint32_t _exp = 0;
    uint8_t _level = 1;
    bool _meditating = false;
    bool _is_ghost = false;
    uint8_t _cls = 3;             // por defecto guerrero (sin hechizos)
    uint8_t _eq_weapon_item = 0;  // item_id del arma equipada

    // Referencia al inventario (para usar poción con atajo)
    const uint8_t* _inv_ref = nullptr;
    int _inv_size = 0;

    bool _inv_clicked = false;
    bool _help_visible = false;

    // Regiones clickeables calculadas por el último render(); handle_event()
    // las usa para resolver clicks (ver StatsPanelRenderer.h).
    StatsLayout _layout;
};
