#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include "../../common/protocol/dtos.h"
#include "../../common/protocol/protocol.h"
#include "../../common/queue.h"
#include "../net/Command.h"
#include "InventoryRenderer.h"

class GameAudioService;

// Dueño de los datos del inventario (qué item hay en cada slot, qué hay
// equipado) y de la interacción de mouse (selección, drag&drop, botones).
// El dibujado en sí y la metadata de items (nombre/abreviatura/categoría)
// viven en InventoryRenderer, usada por composición — ver render().
class InventoryPanel {
public:
    InventoryPanel(SDL2pp::Renderer& renderer, const std::string& font_path,
                   int font_size = 11, GameAudioService* audio = nullptr);
    ~InventoryPanel();

    void set_visible(bool v) { _visible = v; }
    void toggle()            { _visible = !_visible; }
    bool is_visible() const  { return _visible; }
    int  selected_slot() const { return _selected_slot; }

    void update(const uint8_t inventory[SnapshotDTO::INVENTORY_SIZE],
                uint8_t equipped_weapon_slot, uint8_t equipped_armor_slot,
                uint8_t equipped_helmet_slot, uint8_t equipped_shield_slot);

    void register_item_texture(uint8_t item_id, SDL_Texture* tex);

    bool handle_event(const SDL_Event& e, Queue<Command>* cmd_queue = nullptr);
    void render(int screen_w, int screen_h);

private:
    static constexpr int INV_SIZE = SnapshotDTO::INVENTORY_SIZE;

    SDL2pp::Renderer& _renderer;
    GameAudioService* _audio     = nullptr;
    TTF_Font*         _font      = nullptr;
    TTF_Font*         _font_sm   = nullptr;
    int               _font_size;
    bool              _visible   = false;

    // Construido en el cuerpo del constructor (necesita _font/_font_sm ya
    // abiertos), por eso es optional en vez de miembro directo.
    std::optional<InventoryRenderer> _render_impl;

    uint8_t _inventory[INV_SIZE] {};
    uint8_t _eq_wpn  = 0;
    uint8_t _eq_arm  = 0;
    uint8_t _eq_helm = 0;
    uint8_t _eq_shld = 0;

    int _selected_slot = -1;
    int _drag_from_slot = -1;

    // Regiones clickeables calculadas por el último render(); handle_event()
    // las usa para resolver clicks (ver InventoryRenderer.h).
    InventoryLayout _layout;

    std::unordered_map<uint8_t, SDL_Texture*> _item_textures;
};
