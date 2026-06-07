#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cstdint>
#include <unordered_map>
#include "../../common/protocol/dtos.h"
#include "../../common/protocol/protocol.h"
#include "../../common/queue.h"
#include "../net/Command.h"

class InventoryPanel {
public:
    InventoryPanel(SDL2pp::Renderer& renderer, const std::string& font_path,
                   int font_size = 11);
    ~InventoryPanel();

    void set_visible(bool v) { _visible = v; }
    void toggle()            { _visible = !_visible; }
    bool is_visible() const  { return _visible; }

    void update(const uint8_t inventory[SnapshotDTO::INVENTORY_SIZE],
                uint8_t equipped_wpn, uint8_t equipped_arm,
                uint8_t equipped_helm, uint8_t equipped_shld);

    void register_item_texture(uint8_t item_id, SDL_Texture* tex);

    bool handle_event(const SDL_Event& e, Queue<Command>* cmd_queue = nullptr);
    void render(int screen_w, int screen_h);

private:
    static const char* item_name(uint8_t id);
    static const char* item_abbr(uint8_t id);   // abreviatura para slot
    static const char* item_kind(uint8_t id);

    void draw_text_centered(const std::string& text, int cx, int cy,
                            SDL_Color color, TTF_Font* font);
    void draw_text(const std::string& text, int x, int y,
                   SDL_Color color, TTF_Font* font = nullptr);
    void draw_slot(int x, int y, int size, int idx);
    void draw_equip_row(int x, int y, int w, int h,
                        const char* label, uint8_t item_id);
    void draw_panel_bg(int x, int y, int w, int h);
    void draw_lock_icon(int cx, int cy, int size);

    SDL2pp::Renderer& _renderer;
    TTF_Font*         _font      = nullptr;   // fuente normal (11px)
    TTF_Font*         _font_sm   = nullptr;   // fuente pequeña (9px) para abreviaturas
    int               _font_size;
    bool              _visible   = false;

    static constexpr int INV_SIZE = SnapshotDTO::INVENTORY_SIZE;

    uint8_t _inventory[INV_SIZE] {};
    uint8_t _eq_wpn  = 0;
    uint8_t _eq_arm  = 0;
    uint8_t _eq_helm = 0;
    uint8_t _eq_shld = 0;

    int _selected_slot = -1;

    SDL_Rect _slot_rects[INV_SIZE] {};
    SDL_Rect _close_btn   {};
    SDL_Rect _equip_btn   {};
    SDL_Rect _drop_btn    {};
    SDL_Rect _panel_rect  {};

    std::unordered_map<uint8_t, SDL_Texture*> _item_textures;

    // Constantes de layout de la grilla
    static constexpr int COLS     = 5;
    static constexpr int SLOT_SZ  = 52;   // tamaño de cada slot en px
    static constexpr int SLOT_GAP = 4;    // separación entre slots
};
