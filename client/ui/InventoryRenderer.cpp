#include "InventoryRenderer.h"

#include <cmath>
#include <cstdint>
#include <string>

#include "../../common/protocol/protocol.h"

#include "ItemLabels.h"

InventoryRenderer::InventoryRenderer(SDL2pp::Renderer& renderer, TTF_Font* font, TTF_Font* font_sm,
                                     int font_size):
        _renderer(renderer), _font(font), _font_sm(font_sm), _font_size(font_size) {}

void InventoryRenderer::draw_text(const std::string& text, int x, int y, SDL_Color color,
                                  TTF_Font* font) {
    if (text.empty())
        return;
    TTF_Font* f = font ? font : _font;
    if (!f)
        return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, text.c_str(), color);
    if (!surf)
        return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
    SDL_Rect dst{x, y, surf->w, surf->h};
    SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void InventoryRenderer::draw_text_centered(const std::string& text, int cx, int cy, SDL_Color color,
                                           TTF_Font* font) {
    if (text.empty())
        return;
    TTF_Font* f = font ? font : _font;
    if (!f)
        return;
    int w = 0, h = 0;
    TTF_SizeUTF8(f, text.c_str(), &w, &h);
    draw_text(text, cx - w / 2, cy - h / 2, color, f);
}

void InventoryRenderer::draw_panel_bg(int x, int y, int w, int h) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 0, 0, 0, 180);
    SDL_Rect shadow{x + 4, y + 4, w, h};
    SDL_RenderFillRect(_renderer.Get(), &shadow);

    SDL_SetRenderDrawColor(_renderer.Get(), 22, 18, 14, 248);
    SDL_Rect bg{x, y, w, h};
    SDL_RenderFillRect(_renderer.Get(), &bg);

    SDL_SetRenderDrawColor(_renderer.Get(), 80, 65, 45, 255);
    SDL_RenderDrawRect(_renderer.Get(), &bg);

    SDL_SetRenderDrawColor(_renderer.Get(), 40, 32, 22, 255);
    SDL_Rect inner{x + 1, y + 1, w - 2, h - 2};
    SDL_RenderDrawRect(_renderer.Get(), &inner);
}

void InventoryRenderer::draw_lock_icon(int cx, int cy, int sz) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_Color lc{110, 90, 55, 200};
    SDL_SetRenderDrawColor(_renderer.Get(), lc.r, lc.g, lc.b, lc.a);

    int bw = sz * 6 / 10, bh = sz * 5 / 10;
    int bx = cx - bw / 2, by = cy;
    SDL_Rect body{bx, by, bw, bh};
    SDL_RenderFillRect(_renderer.Get(), &body);

    int aw = sz * 4 / 10;
    int ah = sz * 4 / 10;
    int ay = cy - ah;
    for (int dy = 0; dy <= ah; dy++) {
        float t = static_cast<float>(dy) / ah;
        float rx = aw / 2.0f * std::sqrt(std::max(0.0f, 1.0f - t * t));
        if (rx < 1.5f)
            continue;
        SDL_RenderDrawLine(_renderer.Get(), cx - static_cast<int>(rx), ay + dy,
                           cx - static_cast<int>(rx) + 1, ay + dy);
        SDL_RenderDrawLine(_renderer.Get(), cx + static_cast<int>(rx) - 1, ay + dy,
                           cx + static_cast<int>(rx), ay + dy);
    }

    SDL_SetRenderDrawColor(_renderer.Get(), 22, 18, 14, 255);
    SDL_Rect eye{cx - 1, by + bh / 3, 3, bh / 2};
    SDL_RenderFillRect(_renderer.Get(), &eye);
}

void InventoryRenderer::draw_slot(const InventoryRenderInput& input, int x, int y, int size,
                                  int idx, SDL_Rect& out_slot_rect) {
    out_slot_rect = {x, y, size, size};

    uint8_t item = (*input.inventory)[idx];
    bool is_empty = (item == 0);
    bool is_equipped = !is_empty && (idx == input.eq_wpn || idx == input.eq_arm ||
                                     idx == input.eq_helm || idx == input.eq_shld);
    bool is_selected = (input.selected_slot == idx);

    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);

    if (is_selected) {
        SDL_SetRenderDrawColor(_renderer.Get(), 80, 62, 20, 255);
    } else if (is_equipped) {
        SDL_SetRenderDrawColor(_renderer.Get(), 18, 50, 18, 255);
    } else {
        SDL_SetRenderDrawColor(_renderer.Get(), 28, 22, 16, 255);
    }
    SDL_Rect slot_r{x, y, size, size};
    SDL_RenderFillRect(_renderer.Get(), &slot_r);

    SDL_SetRenderDrawColor(_renderer.Get(), 45, 38, 28, 60);
    SDL_Rect top_stripe{x, y, size, size / 3};
    SDL_RenderFillRect(_renderer.Get(), &top_stripe);

    SDL_SetRenderDrawColor(_renderer.Get(), 10, 8, 5, 255);
    SDL_RenderDrawRect(_renderer.Get(), &slot_r);

    SDL_Color hi_color = is_selected ? SDL_Color{200, 160, 40, 200} :
                         is_equipped ? SDL_Color{60, 160, 60, 180} :
                                       SDL_Color{55, 45, 32, 180};
    SDL_SetRenderDrawColor(_renderer.Get(), hi_color.r, hi_color.g, hi_color.b, hi_color.a);
    SDL_RenderDrawLine(_renderer.Get(), x + 1, y + 1, x + size - 2, y + 1);
    SDL_RenderDrawLine(_renderer.Get(), x + 1, y + 1, x + 1, y + size - 2);

    SDL_SetRenderDrawColor(_renderer.Get(), 8, 6, 4, 200);
    SDL_RenderDrawLine(_renderer.Get(), x + 1, y + size - 2, x + size - 2, y + size - 2);
    SDL_RenderDrawLine(_renderer.Get(), x + size - 2, y + 1, x + size - 2, y + size - 2);

    if (is_empty) {
        return;
    }

    int cx = x + size / 2;
    int cy = y + size / 2;

    auto it = input.item_textures->find(item);
    if (it != input.item_textures->end() && it->second) {
        int img_size = size - 10;
        SDL_Rect dst{x + 5, y + 5, img_size, img_size};
        int tex_w = 0, tex_h = 0;
        SDL_QueryTexture(it->second, nullptr, nullptr, &tex_w, &tex_h);
        SDL_Rect icon_src{0, 192, 48, 64};
        SDL_Rect* src = (tex_w == 256 && tex_h == 256) ? &icon_src : nullptr;
        SDL_RenderCopy(_renderer.Get(), it->second, src, &dst);
    } else {
        const char* abbr = ItemLabels::abbr(item);
        if (abbr) {
            SDL_Color text_c = is_equipped ? SDL_Color{120, 230, 120, 255} :
                               is_selected ? SDL_Color{255, 220, 100, 255} :
                                             SDL_Color{210, 200, 175, 255};
            draw_text_centered(std::string(abbr), cx, cy - 2, text_c, _font_sm);
        }
    }

    if (is_equipped) {
        SDL_SetRenderDrawColor(_renderer.Get(), 30, 100, 30, 220);
        SDL_Rect badge{x + size - 14, y + 1, 13, 13};
        SDL_RenderFillRect(_renderer.Get(), &badge);
        SDL_Color eq_c{100, 220, 100, 255};
        draw_text("E", x + size - 11, y + 1, eq_c, _font_sm);
    }
}

void InventoryRenderer::draw_equip_row(const InventoryRenderInput& input, int x, int y, int w,
                                       int h, const char* label, uint8_t slot_idx) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);

    bool has_item = (slot_idx != 0xFF && slot_idx < InventoryRenderInput::INV_SIZE &&
                     (*input.inventory)[slot_idx] != 0);
    uint8_t item_id = has_item ? (*input.inventory)[slot_idx] : 0;

    if (has_item) {
        SDL_SetRenderDrawColor(_renderer.Get(), 16, 45, 16, 240);
    } else {
        SDL_SetRenderDrawColor(_renderer.Get(), 20, 16, 10, 230);
    }
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(_renderer.Get(), &r);

    SDL_Color border_c = has_item ? SDL_Color{55, 130, 55, 200} : SDL_Color{45, 38, 28, 180};
    SDL_SetRenderDrawColor(_renderer.Get(), border_c.r, border_c.g, border_c.b, border_c.a);
    SDL_RenderDrawRect(_renderer.Get(), &r);

    SDL_SetRenderDrawColor(_renderer.Get(), 60, 50, 35, 100);
    SDL_RenderDrawLine(_renderer.Get(), x + 1, y + 1, x + w - 2, y + 1);

    SDL_Color lbl_c{120, 105, 75, 255};
    draw_text(label, x + 6, y + (h - _font_size) / 2, lbl_c);

    const char* name = has_item ? ItemLabels::name(item_id) : nullptr;
    SDL_Color name_c = has_item ? SDL_Color{160, 230, 160, 255} : SDL_Color{55, 50, 40, 255};
    std::string display = name ? std::string(name) : std::string("—");

    int lbl_w = 0, dummy = 0;
    if (_font)
        TTF_SizeUTF8(_font, label, &lbl_w, &dummy);
    draw_text(display, x + 6 + lbl_w + 10, y + (h - _font_size) / 2, name_c);
}

InventoryLayout InventoryRenderer::render(int screen_w, int screen_h,
                                          const InventoryRenderInput& input) {
    InventoryLayout layout;

    const int rows = (InventoryLayout::INV_SIZE + COLS - 1) / COLS;
    const int grid_width = COLS * SLOT_SZ + (COLS - 1) * SLOT_GAP;
    const int padding = 14;
    const int title_height = _font_size + 10;
    const int equip_row_height = 24;
    const int equip_section_height = 4 * (equip_row_height + 3) + 8;
    const int grid_height = rows * SLOT_SZ + (rows - 1) * SLOT_GAP;
    const int label_height = _font_size + 6;
    const int button_height = 22;
    const int action_height = (_font_size + 4) + 4 + button_height + 8;
    const int hint_height = _font_size + 8;

    const int modal_width = grid_width + padding * 2;
    const int modal_height = title_height + equip_section_height + label_height + grid_height +
                             action_height + hint_height + padding * 2;

    const int panel_x = (screen_w - modal_width) / 2;
    const int panel_y = (screen_h - modal_height) / 2;

    layout.panel_rect = {panel_x, panel_y, modal_width, modal_height};

    draw_panel_bg(panel_x, panel_y, modal_width, modal_height);

    SDL_Color gold{210, 175, 55, 255};
    SDL_Color section{150, 125, 75, 255};
    SDL_Color white{230, 220, 200, 255};
    SDL_Color dimgray{90, 85, 70, 255};

    int cy = panel_y + padding;

    draw_text("INVENTARIO", panel_x + padding, cy, gold);

    layout.close_btn = {panel_x + modal_width - 22, panel_y + 6, 16, 16};
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 120, 25, 25, 230);
    SDL_RenderFillRect(_renderer.Get(), &layout.close_btn);
    SDL_SetRenderDrawColor(_renderer.Get(), 190, 55, 55, 255);
    SDL_RenderDrawRect(_renderer.Get(), &layout.close_btn);
    draw_text_centered("X", layout.close_btn.x + 8, layout.close_btn.y + 8, white, _font_sm);

    cy += title_height;
    SDL_SetRenderDrawColor(_renderer.Get(), 70, 58, 38, 180);
    SDL_RenderDrawLine(_renderer.Get(), panel_x + padding, cy - 2, panel_x + modal_width - padding,
                       cy - 2);

    draw_text("Equipado", panel_x + padding, cy, section);
    cy += _font_size + 4;

    struct EqEntry {
        const char* label;
        uint8_t slot_idx;
    };
    EqEntry eq_entries[] = {
            {"Arma", input.eq_wpn},
            {"Armadura", input.eq_arm},
            {"Casco", input.eq_helm},
            {"Escudo", input.eq_shld},
    };
    for (const EqEntry& entry: eq_entries) {
        draw_equip_row(input, panel_x + padding, cy, grid_width, equip_row_height, entry.label,
                       entry.slot_idx);
        cy += equip_row_height + 3;
    }
    cy += 8;

    SDL_SetRenderDrawColor(_renderer.Get(), 70, 58, 38, 180);
    SDL_RenderDrawLine(_renderer.Get(), panel_x + padding, cy - 2, panel_x + modal_width - padding,
                       cy - 2);

    draw_text("Mochila", panel_x + padding, cy, section);
    cy += label_height;

    const int grid_x = panel_x + padding;

    for (int i = 0; i < InventoryLayout::INV_SIZE; i++) {
        int col = i % COLS;
        int row = i / COLS;
        int sx = grid_x + col * (SLOT_SZ + SLOT_GAP);
        int sy = cy + row * (SLOT_SZ + SLOT_GAP);
        draw_slot(input, sx, sy, SLOT_SZ, i, layout.slot_rects[i]);
    }

    cy += grid_height + 8;

    if (input.selected_slot >= 0 && input.selected_slot < InventoryLayout::INV_SIZE &&
        (*input.inventory)[input.selected_slot] != 0) {

        uint8_t item_id = (*input.inventory)[input.selected_slot];
        const char* name = ItemLabels::name(item_id);
        const char* kind = ItemLabels::kind(item_id);

        if (name)
            draw_text(name, panel_x + padding, cy, white);
        if (kind) {
            int nw = 0, nh = 0;
            if (_font && name)
                TTF_SizeUTF8(_font, name, &nw, &nh);
            draw_text(std::string(" (") + kind + ")", panel_x + padding + nw, cy, dimgray);
        }
        cy += _font_size + 4 + 4;

        const int button_width = (grid_width - 6) / 2;

        const bool is_potion = (item_id == static_cast<uint8_t>(ItemId::HEALTH_POTION) ||
                                item_id == static_cast<uint8_t>(ItemId::MANA_POTION));

        bool is_equipped =
                (input.selected_slot == input.eq_wpn || input.selected_slot == input.eq_arm ||
                 input.selected_slot == input.eq_helm || input.selected_slot == input.eq_shld);

        const char* btn_label = is_potion ? "Usar" : is_equipped ? "Desequipar" : "Equipar";

        layout.equip_btn = {panel_x + padding, cy, button_width, button_height};
        if (is_equipped) {
            SDL_SetRenderDrawColor(_renderer.Get(), 70, 50, 18, 230);
            SDL_RenderFillRect(_renderer.Get(), &layout.equip_btn);
            SDL_SetRenderDrawColor(_renderer.Get(), 170, 130, 50, 200);
            SDL_RenderDrawRect(_renderer.Get(), &layout.equip_btn);
            draw_text_centered(btn_label, layout.equip_btn.x + button_width / 2,
                               layout.equip_btn.y + button_height / 2,
                               SDL_Color{235, 200, 110, 255}, _font_sm);
        } else {
            SDL_SetRenderDrawColor(_renderer.Get(), 25, 70, 25, 230);
            SDL_RenderFillRect(_renderer.Get(), &layout.equip_btn);
            SDL_SetRenderDrawColor(_renderer.Get(), 65, 155, 65, 200);
            SDL_RenderDrawRect(_renderer.Get(), &layout.equip_btn);
            draw_text_centered(btn_label, layout.equip_btn.x + button_width / 2,
                               layout.equip_btn.y + button_height / 2,
                               SDL_Color{160, 235, 160, 255}, _font_sm);
        }

        layout.drop_btn = {panel_x + padding + button_width + 6, cy, button_width, button_height};
        SDL_SetRenderDrawColor(_renderer.Get(), 70, 18, 18, 230);
        SDL_RenderFillRect(_renderer.Get(), &layout.drop_btn);
        SDL_SetRenderDrawColor(_renderer.Get(), 155, 55, 55, 200);
        SDL_RenderDrawRect(_renderer.Get(), &layout.drop_btn);
        draw_text_centered("Tirar", layout.drop_btn.x + button_width / 2,
                           layout.drop_btn.y + button_height / 2, SDL_Color{235, 150, 150, 255},
                           _font_sm);

    } else {
        layout.equip_btn = {};
        layout.drop_btn = {};
        draw_text("Selecciona un objeto para ver opciones", panel_x + padding, cy, dimgray);
        cy += _font_size + 4;
    }

    draw_text("ESC para cerrar | Click para seleccionar", panel_x + padding,
              panel_y + modal_height - hint_height, dimgray);

    return layout;
}
