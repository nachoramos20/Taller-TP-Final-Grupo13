// Patrón aplicado: Factory + tabla de lookup (en vez de 2 switches de 26
// casos + una cadena de 8 if/else por rango) para la metadata de items que
// hoy no expone Items::get() al cliente (nombre largo, abreviatura de 3
// letras, categoría). Agregar un item nuevo es agregar una fila a cada
// tabla, no un case más en cada switch.
#include "InventoryPanel.h"
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <unordered_map>
#include <vector>
#include "../audio/GameAudioService.h"

namespace {
    const std::unordered_map<uint8_t, const char*>& item_name_table() {
        using I = ItemId;
        static const std::unordered_map<uint8_t, const char*> table = {
            {static_cast<uint8_t>(I::SWORD),               "Espada"},
            {static_cast<uint8_t>(I::AXE),                 "Hacha"},
            {static_cast<uint8_t>(I::HAMMER),              "Martillo"},
            {static_cast<uint8_t>(I::SIMPLE_BOW),          "Arco Simple"},
            {static_cast<uint8_t>(I::COMPOUND_BOW),        "Arco Compuesto"},
            {static_cast<uint8_t>(I::ELVEN_FLUTE),         "Flauta Élfica"},
            {static_cast<uint8_t>(I::ASH_STICK),           "Vara de Fresno"},
            {static_cast<uint8_t>(I::NUDOSO_STAFF),        "Báculo Nudoso"},
            {static_cast<uint8_t>(I::GEMMED_STAFF),        "Báculo Engarzado"},
            {static_cast<uint8_t>(I::LEATHER_ARMOR),       "Túnica de Clérigo"},
            {static_cast<uint8_t>(I::CLERIC_BLACK_ARMOR),  "Túnica Negra"},
            {static_cast<uint8_t>(I::MAGE_COMMON_ARMOR),   "Ropa de Mago"},
            {static_cast<uint8_t>(I::MAGE_ROYAL_ARMOR),    "Mago Real"},
            {static_cast<uint8_t>(I::PLATE_ARMOR),         "Guerrero Ejecutor"},
            {static_cast<uint8_t>(I::WARRIOR_EPIC_ARMOR),  "Guerrero Épico"},
            {static_cast<uint8_t>(I::PALADIN_MAGIC_ARMOR), "Paladín Mágico"},
            {static_cast<uint8_t>(I::PALADIN_ROYAL_ARMOR), "Paladín Real"},
            {static_cast<uint8_t>(I::HOOD),                "Capucha"},
            {static_cast<uint8_t>(I::IRON_HELMET),         "Casco de Hierro"},
            {static_cast<uint8_t>(I::MAGIC_HAT),           "Sombrero Mágico"},
            {static_cast<uint8_t>(I::TURTLE_SHIELD),       "Escudo Tortuga"},
            {static_cast<uint8_t>(I::IRON_SHIELD),         "Escudo de Hierro"},
            {static_cast<uint8_t>(I::BOCA_SHIELD),         "Escudo Boca"},
            {static_cast<uint8_t>(I::HEALTH_POTION),       "Poción de Vida"},
            {static_cast<uint8_t>(I::MANA_POTION),         "Poción de Maná"},
            {static_cast<uint8_t>(I::GOLD_PILE),           "Monedas de Oro"},
        };
        return table;
    }

    const std::unordered_map<uint8_t, const char*>& item_abbr_table() {
        using I = ItemId;
        static const std::unordered_map<uint8_t, const char*> table = {
            {static_cast<uint8_t>(I::SWORD),               "ESP"},
            {static_cast<uint8_t>(I::AXE),                 "HAC"},
            {static_cast<uint8_t>(I::HAMMER),              "MAR"},
            {static_cast<uint8_t>(I::SIMPLE_BOW),          "ARS"},
            {static_cast<uint8_t>(I::COMPOUND_BOW),        "ARC"},
            {static_cast<uint8_t>(I::ELVEN_FLUTE),         "FLA"},
            {static_cast<uint8_t>(I::ASH_STICK),           "VFR"},
            {static_cast<uint8_t>(I::NUDOSO_STAFF),        "BNU"},
            {static_cast<uint8_t>(I::GEMMED_STAFF),        "BAE"},
            {static_cast<uint8_t>(I::LEATHER_ARMOR),       "TCL"},
            {static_cast<uint8_t>(I::CLERIC_BLACK_ARMOR),  "TNE"},
            {static_cast<uint8_t>(I::MAGE_COMMON_ARMOR),   "RMA"},
            {static_cast<uint8_t>(I::MAGE_ROYAL_ARMOR),    "RMR"},
            {static_cast<uint8_t>(I::PLATE_ARMOR),         "GEJ"},
            {static_cast<uint8_t>(I::WARRIOR_EPIC_ARMOR),  "GEP"},
            {static_cast<uint8_t>(I::PALADIN_MAGIC_ARMOR), "PMA"},
            {static_cast<uint8_t>(I::PALADIN_ROYAL_ARMOR), "PRE"},
            {static_cast<uint8_t>(I::HOOD),                "CAP"},
            {static_cast<uint8_t>(I::IRON_HELMET),         "CHI"},
            {static_cast<uint8_t>(I::MAGIC_HAT),           "SOM"},
            {static_cast<uint8_t>(I::TURTLE_SHIELD),       "EST"},
            {static_cast<uint8_t>(I::IRON_SHIELD),         "ESI"},
            {static_cast<uint8_t>(I::BOCA_SHIELD),         "ESB"},
            {static_cast<uint8_t>(I::HEALTH_POTION),       "PVI"},
            {static_cast<uint8_t>(I::MANA_POTION),         "PMA"},
            {static_cast<uint8_t>(I::GOLD_PILE),           "ORO"},
        };
        return table;
    }

    struct ItemKindRange { uint8_t lo, hi; const char* label; };

    const std::vector<ItemKindRange>& item_kind_ranges() {
        using I = ItemId;
        static const std::vector<ItemKindRange> ranges = {
            {static_cast<uint8_t>(I::SWORD),          static_cast<uint8_t>(I::HAMMER),             "Arma Melee"},
            {static_cast<uint8_t>(I::SIMPLE_BOW),     static_cast<uint8_t>(I::COMPOUND_BOW),       "Arma Ranged"},
            {static_cast<uint8_t>(I::ELVEN_FLUTE),    static_cast<uint8_t>(I::NUDOSO_STAFF),       "Arma Mágica"},
            {static_cast<uint8_t>(I::LEATHER_ARMOR),  static_cast<uint8_t>(I::PALADIN_ROYAL_ARMOR),"Armadura"},
            {static_cast<uint8_t>(I::HOOD),           static_cast<uint8_t>(I::MAGIC_HAT),          "Casco"},
            {static_cast<uint8_t>(I::TURTLE_SHIELD),  static_cast<uint8_t>(I::BOCA_SHIELD),        "Escudo"},
            {static_cast<uint8_t>(I::HEALTH_POTION),  static_cast<uint8_t>(I::MANA_POTION),        "Poción"},
            {static_cast<uint8_t>(I::GOLD_PILE),      static_cast<uint8_t>(I::GOLD_PILE),          "Oro"},
        };
        return ranges;
    }
}

const char* InventoryPanel::item_name(uint8_t id) {
    auto it = item_name_table().find(id);
    return it != item_name_table().end() ? it->second : nullptr;
}

const char* InventoryPanel::item_abbr(uint8_t id) {
    auto it = item_abbr_table().find(id);
    return it != item_abbr_table().end() ? it->second : "???";
}

const char* InventoryPanel::item_kind(uint8_t id) {
    if (id == 0) return nullptr;
    for (const ItemKindRange& range : item_kind_ranges())
        if (id >= range.lo && id <= range.hi) return range.label;
    return nullptr;
}

InventoryPanel::InventoryPanel(SDL2pp::Renderer& renderer,
                               const std::string& font_path, int font_size,
                               GameAudioService* audio)
    : _renderer(renderer), _audio(audio), _font_size(font_size) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0)
            throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    _font = TTF_OpenFont(font_path.c_str(), font_size);
    if (!_font)
        throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());

    _font_sm = TTF_OpenFont(font_path.c_str(), std::max(8, font_size - 2));

    std::memset(_inventory, 0, sizeof(_inventory));
}

InventoryPanel::~InventoryPanel() {
    if (_font)    TTF_CloseFont(_font);
    if (_font_sm) TTF_CloseFont(_font_sm);
}

void InventoryPanel::update(const uint8_t inventory[INV_SIZE],
                            uint8_t equipped_weapon_slot, uint8_t equipped_armor_slot,
                            uint8_t equipped_helmet_slot, uint8_t equipped_shield_slot) {
    std::memcpy(_inventory, inventory, INV_SIZE);
    _eq_wpn  = equipped_weapon_slot;
    _eq_arm  = equipped_armor_slot;
    _eq_helm = equipped_helmet_slot;
    _eq_shld = equipped_shield_slot;
}

void InventoryPanel::register_item_texture(uint8_t item_id, SDL_Texture* tex) {
    _item_textures[item_id] = tex;
}

bool InventoryPanel::handle_event(const SDL_Event& e, Queue<Command>* cmd_queue) {
    if (!_visible) return false;

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        _visible = false;
        return true;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;

        bool (*in)(int, int, const SDL_Rect&) = [](int mx, int my, const SDL_Rect& r) {
            return mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
        };

        if (in(mx, my, _close_btn)) {
            if (_audio) _audio->click();
            _visible = false;
            return true;
        }

        if (_selected_slot >= 0 && _inventory[_selected_slot] != 0) {
            if (in(mx, my, _equip_btn)) {
                if (_audio) _audio->click();
                if (cmd_queue) {
                    uint8_t item_id = _inventory[_selected_slot];
                    bool is_potion = (item_id == 40 || item_id == 41);
                    if (is_potion) {
                        cmd_queue->push(Command::use_item(static_cast<uint8_t>(_selected_slot)));
                    } else if (_selected_slot == _eq_wpn)  cmd_queue->push(Command::unequip(EquipSlot::WEAPON));
                    else if (_selected_slot == _eq_arm)  cmd_queue->push(Command::unequip(EquipSlot::ARMOR));
                    else if (_selected_slot == _eq_helm) cmd_queue->push(Command::unequip(EquipSlot::HELMET));
                    else if (_selected_slot == _eq_shld) cmd_queue->push(Command::unequip(EquipSlot::SHIELD));
                    else {
                        cmd_queue->push(Command::equip(static_cast<uint8_t>(_selected_slot)));
                    }
                }
                return true;
            }
            if (in(mx, my, _drop_btn)) {
                if (_audio) _audio->click();
                if (cmd_queue) cmd_queue->push(Command::drop(
                    static_cast<uint8_t>(_selected_slot)));
                return true;
            }
        }

        for (int i = 0; i < INV_SIZE; i++) {
            if (in(mx, my, _slot_rects[i])) {
                if (_audio) _audio->click();
                _selected_slot  = (_selected_slot == i) ? -1 : i;
                _drag_from_slot = i;  
                return true;
            }
        }

        if (in(mx, my, _panel_rect)) return true;
    }

    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        int from = _drag_from_slot;
        _drag_from_slot = -1;
        if (from < 0 || _inventory[from] == 0) return false;

        int mx = e.button.x, my = e.button.y;
        bool (*in)(int, int, const SDL_Rect&) = [](int mx, int my, const SDL_Rect& r) {
            return mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
        };

        for (int i = 0; i < INV_SIZE; i++) {
            if (i != from && in(mx, my, _slot_rects[i])) {
                if (_audio) _audio->click();
                if (cmd_queue) cmd_queue->push(Command::move_item(
                    static_cast<uint8_t>(from), static_cast<uint8_t>(i)));
                return true;
            }
        }
    }
    return false;
}

void InventoryPanel::draw_text(const std::string& text, int x, int y,
                               SDL_Color color, TTF_Font* font) {
    if (text.empty()) return;
    TTF_Font* f = font ? font : _font;
    if (!f) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
    SDL_Rect dst{ x, y, surf->w, surf->h };
    SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void InventoryPanel::draw_text_centered(const std::string& text, int cx, int cy,
                                        SDL_Color color, TTF_Font* font) {
    if (text.empty()) return;
    TTF_Font* f = font ? font : _font;
    if (!f) return;
    int w = 0, h = 0;
    TTF_SizeUTF8(f, text.c_str(), &w, &h);
    draw_text(text, cx - w / 2, cy - h / 2, color, f);
}

void InventoryPanel::draw_panel_bg(int x, int y, int w, int h) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 0, 0, 0, 180);
    SDL_Rect shadow{ x + 4, y + 4, w, h };
    SDL_RenderFillRect(_renderer.Get(), &shadow);

    SDL_SetRenderDrawColor(_renderer.Get(), 22, 18, 14, 248);
    SDL_Rect bg{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &bg);

    SDL_SetRenderDrawColor(_renderer.Get(), 80, 65, 45, 255);
    SDL_RenderDrawRect(_renderer.Get(), &bg);

    SDL_SetRenderDrawColor(_renderer.Get(), 40, 32, 22, 255);
    SDL_Rect inner{ x + 1, y + 1, w - 2, h - 2 };
    SDL_RenderDrawRect(_renderer.Get(), &inner);
}

void InventoryPanel::draw_lock_icon(int cx, int cy, int sz) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_Color lc{ 110, 90, 55, 200 };
    SDL_SetRenderDrawColor(_renderer.Get(), lc.r, lc.g, lc.b, lc.a);

    int bw = sz * 6 / 10, bh = sz * 5 / 10;
    int bx = cx - bw / 2, by = cy;
    SDL_Rect body{ bx, by, bw, bh };
    SDL_RenderFillRect(_renderer.Get(), &body);

    int aw = sz * 4 / 10;
    int ah = sz * 4 / 10;
    int ay = cy - ah;
    for (int dy = 0; dy <= ah; dy++) {
        float t = static_cast<float>(dy) / ah;
        float rx = aw / 2.0f * std::sqrt(std::max(0.0f, 1.0f - t * t));
        if (rx < 1.5f) continue;
        SDL_RenderDrawLine(_renderer.Get(),
            cx - static_cast<int>(rx), ay + dy,
            cx - static_cast<int>(rx) + 1, ay + dy);
        SDL_RenderDrawLine(_renderer.Get(),
            cx + static_cast<int>(rx) - 1, ay + dy,
            cx + static_cast<int>(rx), ay + dy);
    }

    SDL_SetRenderDrawColor(_renderer.Get(), 22, 18, 14, 255);
    SDL_Rect eye{ cx - 1, by + bh / 3, 3, bh / 2 };
    SDL_RenderFillRect(_renderer.Get(), &eye);
}

void InventoryPanel::draw_slot(int x, int y, int size, int idx) {
    _slot_rects[idx] = { x, y, size, size };

    uint8_t item     = _inventory[idx];
    bool is_empty    = (item == 0);
    bool is_equipped = !is_empty && (idx == _eq_wpn  || idx == _eq_arm ||
                                     idx == _eq_helm || idx == _eq_shld);
    bool is_selected = (_selected_slot == idx);

    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);

    if (is_selected) {
        SDL_SetRenderDrawColor(_renderer.Get(), 80, 62, 20, 255);
    } else if (is_equipped) {
        SDL_SetRenderDrawColor(_renderer.Get(), 18, 50, 18, 255);
    } else {
        SDL_SetRenderDrawColor(_renderer.Get(), 28, 22, 16, 255);
    }
    SDL_Rect slot_r{ x, y, size, size };
    SDL_RenderFillRect(_renderer.Get(), &slot_r);

    SDL_SetRenderDrawColor(_renderer.Get(), 45, 38, 28, 60);
    SDL_Rect top_stripe{ x, y, size, size / 3 };
    SDL_RenderFillRect(_renderer.Get(), &top_stripe);

    SDL_SetRenderDrawColor(_renderer.Get(), 10, 8, 5, 255);
    SDL_RenderDrawRect(_renderer.Get(), &slot_r);

    SDL_Color hi_color = is_selected ? SDL_Color{200, 160, 40, 200}
                       : is_equipped ? SDL_Color{60, 160, 60, 180}
                       :               SDL_Color{55, 45, 32, 180};
    SDL_SetRenderDrawColor(_renderer.Get(), hi_color.r, hi_color.g, hi_color.b, hi_color.a);
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+1, x+size-2, y+1);
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+1, x+1, y+size-2);

    SDL_SetRenderDrawColor(_renderer.Get(), 8, 6, 4, 200);
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+size-2, x+size-2, y+size-2);
    SDL_RenderDrawLine(_renderer.Get(), x+size-2, y+1, x+size-2, y+size-2);

    if (is_empty) {
        return;
    }

    int cx = x + size / 2;
    int cy = y + size / 2;

    std::unordered_map<uint8_t, SDL_Texture*>::iterator it = _item_textures.find(item);
    if (it != _item_textures.end() && it->second) {
        int img_size = size - 10;
        SDL_Rect dst{ x + 5, y + 5, img_size, img_size };
        int tex_w = 0, tex_h = 0;
        SDL_QueryTexture(it->second, nullptr, nullptr, &tex_w, &tex_h);
        SDL_Rect icon_src{ 0, 192, 48, 64 };
        SDL_Rect* src = (tex_w == 256 && tex_h == 256) ? &icon_src : nullptr;
        SDL_RenderCopy(_renderer.Get(), it->second, src, &dst);
    } else {
        const char* abbr = item_abbr(item);
        if (abbr) {
            SDL_Color text_c = is_equipped ? SDL_Color{120, 230, 120, 255}
                             : is_selected ? SDL_Color{255, 220, 100, 255}
                             :               SDL_Color{210, 200, 175, 255};
            draw_text_centered(std::string(abbr), cx, cy - 2, text_c, _font_sm);
        }
    }

    if (is_equipped) {
        SDL_SetRenderDrawColor(_renderer.Get(), 30, 100, 30, 220);
        SDL_Rect badge{ x + size - 14, y + 1, 13, 13 };
        SDL_RenderFillRect(_renderer.Get(), &badge);
        SDL_Color eq_c{ 100, 220, 100, 255 };
        draw_text("E", x + size - 11, y + 1, eq_c, _font_sm);
    }
}

void InventoryPanel::draw_equip_row(int x, int y, int w, int h,
                                     const char* label, uint8_t slot_idx) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);

    bool has_item = (slot_idx != 0xFF && slot_idx < INV_SIZE && _inventory[slot_idx] != 0);
    uint8_t item_id = has_item ? _inventory[slot_idx] : 0;

    if (has_item) {
        SDL_SetRenderDrawColor(_renderer.Get(), 16, 45, 16, 240);
    } else {
        SDL_SetRenderDrawColor(_renderer.Get(), 20, 16, 10, 230);
    }
    SDL_Rect r{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &r);

    SDL_Color border_c = has_item ? SDL_Color{55, 130, 55, 200}
                                  : SDL_Color{45, 38, 28, 180};
    SDL_SetRenderDrawColor(_renderer.Get(), border_c.r, border_c.g, border_c.b, border_c.a);
    SDL_RenderDrawRect(_renderer.Get(), &r);

    SDL_SetRenderDrawColor(_renderer.Get(), 60, 50, 35, 100);
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+1, x+w-2, y+1);

    SDL_Color lbl_c{ 120, 105, 75, 255 };
    draw_text(label, x + 6, y + (h - _font_size) / 2, lbl_c);

    const char* name = has_item ? item_name(item_id) : nullptr;
    SDL_Color name_c = has_item ? SDL_Color{160, 230, 160, 255}
                                : SDL_Color{ 55,  50,  40, 255};
    std::string display = name ? std::string(name) : std::string("—");

    int lbl_w = 0, dummy = 0;
    if (_font) TTF_SizeUTF8(_font, label, &lbl_w, &dummy);
    draw_text(display, x + 6 + lbl_w + 10, y + (h - _font_size) / 2, name_c);
}

void InventoryPanel::render(int screen_w, int screen_h) {
    if (!_visible) return;

    const int rows        = (INV_SIZE + COLS - 1) / COLS;
    const int grid_width  = COLS * SLOT_SZ + (COLS - 1) * SLOT_GAP;
    const int padding     = 14;
    const int title_height = _font_size + 10;
    const int equip_row_height = 24;
    const int equip_section_height = 4 * (equip_row_height + 3) + 8;
    const int grid_height = rows * SLOT_SZ + (rows - 1) * SLOT_GAP;
    const int label_height = _font_size + 6;
    const int button_height = 22;
    const int action_height = (_font_size + 4) + 4 + button_height + 8;
    const int hint_height = _font_size + 8;

    const int modal_width = grid_width + padding * 2;
    const int modal_height = title_height + equip_section_height + label_height +
                       grid_height + action_height + hint_height + padding * 2;

    const int panel_x = (screen_w - modal_width) / 2;
    const int panel_y = (screen_h - modal_height) / 2;

    _panel_rect = { panel_x, panel_y, modal_width, modal_height };

    draw_panel_bg(panel_x, panel_y, modal_width, modal_height);

    SDL_Color gold    { 210, 175,  55, 255 };
    SDL_Color section { 150, 125,  75, 255 };
    SDL_Color white   { 230, 220, 200, 255 };
    SDL_Color dimgray { 90,  85,  70, 255  };

    int cy = panel_y + padding;

    draw_text("INVENTARIO", panel_x + padding, cy, gold);

    _close_btn = { panel_x + modal_width - 22, panel_y + 6, 16, 16 };
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 120, 25, 25, 230);
    SDL_RenderFillRect(_renderer.Get(), &_close_btn);
    SDL_SetRenderDrawColor(_renderer.Get(), 190, 55, 55, 255);
    SDL_RenderDrawRect(_renderer.Get(), &_close_btn);
    draw_text_centered("X", _close_btn.x + 8, _close_btn.y + 8, white, _font_sm);

    cy += title_height;
    SDL_SetRenderDrawColor(_renderer.Get(), 70, 58, 38, 180);
    SDL_RenderDrawLine(_renderer.Get(), panel_x + padding, cy - 2,
                       panel_x + modal_width - padding, cy - 2);

    draw_text("Equipado", panel_x + padding, cy, section);
    cy += _font_size + 4;

    struct EqEntry { const char* label; uint8_t slot_idx; };
    EqEntry eq_entries[] = {
        { "Arma",     _eq_wpn  },
        { "Armadura", _eq_arm  },
        { "Casco",    _eq_helm },
        { "Escudo",   _eq_shld },
    };
    for (const EqEntry& entry : eq_entries) {
        draw_equip_row(panel_x + padding, cy, grid_width, equip_row_height, entry.label, entry.slot_idx);
        cy += equip_row_height + 3;
    }
    cy += 8;

    SDL_SetRenderDrawColor(_renderer.Get(), 70, 58, 38, 180);
    SDL_RenderDrawLine(_renderer.Get(), panel_x + padding, cy - 2,
                       panel_x + modal_width - padding, cy - 2);

    draw_text("Mochila", panel_x + padding, cy, section);
    cy += label_height;

    const int grid_x = panel_x + padding;

    for (int i = 0; i < INV_SIZE; i++) {
        int col = i % COLS;
        int row = i / COLS;
        int sx  = grid_x + col * (SLOT_SZ + SLOT_GAP);
        int sy  = cy  + row * (SLOT_SZ + SLOT_GAP);
        draw_slot(sx, sy, SLOT_SZ, i);
    }

    cy += grid_height + 8;

    if (_selected_slot >= 0 && _selected_slot < INV_SIZE
        && _inventory[_selected_slot] != 0) {

        uint8_t item_id = _inventory[_selected_slot];
        const char* name = item_name(item_id);
        const char* kind = item_kind(item_id);

        if (name) draw_text(name, panel_x + padding, cy, white);
        if (kind) {
            int nw = 0, nh = 0;
            if (_font && name) TTF_SizeUTF8(_font, name, &nw, &nh);
            draw_text(std::string(" (") + kind + ")", panel_x + padding + nw, cy, dimgray);
        }
        cy += _font_size + 4 + 4;

        const int button_width = (grid_width - 6) / 2;

        const bool is_potion = (item_id == static_cast<uint8_t>(ItemId::HEALTH_POTION) ||
                                item_id == static_cast<uint8_t>(ItemId::MANA_POTION));

        bool is_equipped = (_selected_slot == _eq_wpn  || _selected_slot == _eq_arm ||
                            _selected_slot == _eq_helm || _selected_slot == _eq_shld);

        const char* btn_label = is_potion    ? "Usar"
                              : is_equipped  ? "Desequipar"
                                             : "Equipar";

        _equip_btn = { panel_x + padding, cy, button_width, button_height };
        if (is_equipped) {
            SDL_SetRenderDrawColor(_renderer.Get(), 70, 50, 18, 230);
            SDL_RenderFillRect(_renderer.Get(), &_equip_btn);
            SDL_SetRenderDrawColor(_renderer.Get(), 170, 130, 50, 200);
            SDL_RenderDrawRect(_renderer.Get(), &_equip_btn);
            draw_text_centered(btn_label,
                               _equip_btn.x + button_width / 2, _equip_btn.y + button_height / 2,
                               SDL_Color{235, 200, 110, 255}, _font_sm);
        } else {
            SDL_SetRenderDrawColor(_renderer.Get(), 25, 70, 25, 230);
            SDL_RenderFillRect(_renderer.Get(), &_equip_btn);
            SDL_SetRenderDrawColor(_renderer.Get(), 65, 155, 65, 200);
            SDL_RenderDrawRect(_renderer.Get(), &_equip_btn);
            draw_text_centered(btn_label,
                               _equip_btn.x + button_width / 2, _equip_btn.y + button_height / 2,
                               SDL_Color{160, 235, 160, 255}, _font_sm);
        }

        _drop_btn = { panel_x + padding + button_width + 6, cy, button_width, button_height };
        SDL_SetRenderDrawColor(_renderer.Get(), 70, 18, 18, 230);
        SDL_RenderFillRect(_renderer.Get(), &_drop_btn);
        SDL_SetRenderDrawColor(_renderer.Get(), 155, 55, 55, 200);
        SDL_RenderDrawRect(_renderer.Get(), &_drop_btn);
        draw_text_centered("Tirar",
                           _drop_btn.x + button_width / 2, _drop_btn.y + button_height / 2,
                           SDL_Color{235, 150, 150, 255}, _font_sm);

    } else {
        _equip_btn = {};
        _drop_btn  = {};
        draw_text("Selecciona un objeto para ver opciones", panel_x + padding, cy, dimgray);
        cy += _font_size + 4;
    }

    draw_text("ESC para cerrar | Click para seleccionar",
              panel_x + padding, panel_y + modal_height - hint_height, dimgray);
}
