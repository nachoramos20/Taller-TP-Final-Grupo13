#pragma once

#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../common/protocol/dtos.h"

// Resultado del render: las regiones clickeables que InventoryPanel
// necesita recordar para resolver handle_event() en el frame siguiente
// (selección de slot, drag&drop, botones equipar/tirar/cerrar). Se
// calculan durante el layout del render, así que el renderer es quien
// las produce; InventoryPanel las guarda porque son estado de
// interacción, no de dibujado.
struct InventoryLayout {
    static constexpr int INV_SIZE = SnapshotDTO::INVENTORY_SIZE;
    SDL_Rect slot_rects[INV_SIZE]{};
    SDL_Rect close_btn{};
    SDL_Rect equip_btn{};
    SDL_Rect drop_btn{};
    SDL_Rect panel_rect{};
};

// Qué dibujar: una copia liviana de los datos de InventoryPanel que el
// renderer necesita para ese frame (no posee nada, son punteros/refs a
// estado que sigue viviendo en InventoryPanel).
struct InventoryRenderInput {
    static constexpr int INV_SIZE = SnapshotDTO::INVENTORY_SIZE;
    const uint8_t (*inventory)[INV_SIZE];
    uint8_t eq_wpn, eq_arm, eq_helm, eq_shld;
    int selected_slot;
    const std::unordered_map<uint8_t, SDL_Texture*>* item_textures;
};

// Todo el dibujado del panel de inventario (fondo, grilla de slots, fila
// de equipo, botones, tooltips). Extraída de InventoryPanel, que mezclaba
// esto con el manejo de eventos de mouse y la sincronización de datos del
// inventario. La metadata de items (nombre/abreviatura/categoría) que
// antes vivía acá también se extrajo aparte, a ItemLabels: es lookup de
// datos, no dibujado.
//
// Queda ~340 líneas de .cpp, todavía por encima de las 200: no se
// fragmenta más porque cada draw_* sirve a la misma única responsabilidad
// ("dibujar el panel"), no hay parseo/cálculo/persistencia mezclado — es
// el motivo real para dividir, no el recuento de líneas en sí. Separar
// "primitivas de texto" de "layout del panel" forzaría a draw_slot/
// draw_equip_row a depender de otro objeto solo para llamar a draw_text,
// sin ganar cohesión real.
// datos, no dibujado.
class InventoryRenderer {
public:
    InventoryRenderer(SDL2pp::Renderer& renderer, TTF_Font* font, TTF_Font* font_sm, int font_size);

    InventoryLayout render(int screen_w, int screen_h, const InventoryRenderInput& input);

private:
    void draw_text_centered(const std::string& text, int cx, int cy, SDL_Color color,
                            TTF_Font* font);
    void draw_text(const std::string& text, int x, int y, SDL_Color color,
                   TTF_Font* font = nullptr);
    void draw_slot(const InventoryRenderInput& input, int x, int y, int size, int idx,
                   SDL_Rect& out_slot_rect);
    void draw_equip_row(const InventoryRenderInput& input, int x, int y, int w, int h,
                        const char* label, uint8_t slot_idx);
    void draw_panel_bg(int x, int y, int w, int h);
    void draw_lock_icon(int cx, int cy, int size);

    SDL2pp::Renderer& _renderer;
    TTF_Font* _font;
    TTF_Font* _font_sm;
    int _font_size;

    static constexpr int COLS = 5;
    static constexpr int SLOT_SZ = 52;
    static constexpr int SLOT_GAP = 4;
};
