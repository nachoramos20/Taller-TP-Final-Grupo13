#include "InventoryPanel.h"
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cmath>
#include "../audio/GameAudioService.h"

//  Datos de items

const char* InventoryPanel::item_name(uint8_t id) {
    switch (id) {
        // Armas cuerpo a cuerpo
        case static_cast<uint8_t>(ItemId::SWORD):              return "Espada";
        case static_cast<uint8_t>(ItemId::AXE):                return "Hacha";
        case static_cast<uint8_t>(ItemId::HAMMER):             return "Martillo";

        // Armas a distancia
        case static_cast<uint8_t>(ItemId::SIMPLE_BOW):         return "Arco Simple";
        case static_cast<uint8_t>(ItemId::COMPOUND_BOW):       return "Arco Compuesto";

        // Armas mágicas
        case static_cast<uint8_t>(ItemId::ELVEN_FLUTE):        return "Flauta Élfica";
        case static_cast<uint8_t>(ItemId::ASH_STICK):          return "Vara de Fresno";
        case static_cast<uint8_t>(ItemId::NUDOSO_STAFF):       return "Báculo Nudoso";
        case static_cast<uint8_t>(ItemId::GEMMED_STAFF):       return "Báculo Engarzado";

        // Armaduras
        case static_cast<uint8_t>(ItemId::LEATHER_ARMOR):      return "Túnica de Clérigo";
        case static_cast<uint8_t>(ItemId::CLERIC_BLACK_ARMOR): return "Túnica Negra";
        case static_cast<uint8_t>(ItemId::MAGE_COMMON_ARMOR):  return "Ropa de Mago";
        case static_cast<uint8_t>(ItemId::MAGE_ROYAL_ARMOR):   return "Mago Real";
        case static_cast<uint8_t>(ItemId::PLATE_ARMOR):        return "Guerrero Ejecutor";
        case static_cast<uint8_t>(ItemId::WARRIOR_EPIC_ARMOR): return "Guerrero Épico";
        case static_cast<uint8_t>(ItemId::PALADIN_MAGIC_ARMOR): return "Paladín Mágico";
        case static_cast<uint8_t>(ItemId::PALADIN_ROYAL_ARMOR): return "Paladín Real";
        
        // Cascos
        case static_cast<uint8_t>(ItemId::HOOD):               return "Capucha";
        case static_cast<uint8_t>(ItemId::IRON_HELMET):        return "Casco de Hierro";
        case static_cast<uint8_t>(ItemId::MAGIC_HAT):          return "Sombrero Mágico";
        
        // Escudos
        case static_cast<uint8_t>(ItemId::TURTLE_SHIELD):      return "Escudo Tortuga";
        case static_cast<uint8_t>(ItemId::IRON_SHIELD):        return "Escudo de Hierro";
        case static_cast<uint8_t>(ItemId::BOCA_SHIELD):        return "Escudo Boca";
        
        // Pociones
        case static_cast<uint8_t>(ItemId::HEALTH_POTION):      return "Poción de Vida";
        case static_cast<uint8_t>(ItemId::MANA_POTION):        return "Poción de Maná";
        
        // Oro
        case static_cast<uint8_t>(ItemId::GOLD_PILE):          return "Monedas de Oro";
        
        default: return nullptr;
    }
}

// Abreviatura de 3-4 letras para mostrar dentro del slot cuadrado
const char* InventoryPanel::item_abbr(uint8_t id) {
    switch (id) {
        // Armas cuerpo a cuerpo
        case static_cast<uint8_t>(ItemId::SWORD):              return "ESP";
        case static_cast<uint8_t>(ItemId::AXE):                return "HAC";
        case static_cast<uint8_t>(ItemId::HAMMER):             return "MAR";

        // Armas a distancia
        case static_cast<uint8_t>(ItemId::SIMPLE_BOW):         return "ARS";
        case static_cast<uint8_t>(ItemId::COMPOUND_BOW):       return "ARC";

        // Armas mágicas
        case static_cast<uint8_t>(ItemId::ELVEN_FLUTE):        return "FLA";
        case static_cast<uint8_t>(ItemId::ASH_STICK):          return "VFR";
        case static_cast<uint8_t>(ItemId::NUDOSO_STAFF):       return "BNU";
        case static_cast<uint8_t>(ItemId::GEMMED_STAFF):       return "BAE";

        // Armaduras
        case static_cast<uint8_t>(ItemId::LEATHER_ARMOR):      return "TCL";
        case static_cast<uint8_t>(ItemId::CLERIC_BLACK_ARMOR): return "TNE";
        case static_cast<uint8_t>(ItemId::MAGE_COMMON_ARMOR):  return "RMA";
        case static_cast<uint8_t>(ItemId::MAGE_ROYAL_ARMOR):   return "RMR";
        case static_cast<uint8_t>(ItemId::PLATE_ARMOR):        return "GEJ";
        case static_cast<uint8_t>(ItemId::WARRIOR_EPIC_ARMOR): return "GEP";
        case static_cast<uint8_t>(ItemId::PALADIN_MAGIC_ARMOR): return "PMA";
        case static_cast<uint8_t>(ItemId::PALADIN_ROYAL_ARMOR): return "PRE";
        
        // Cascos
        case static_cast<uint8_t>(ItemId::HOOD):               return "CAP";
        case static_cast<uint8_t>(ItemId::IRON_HELMET):        return "CHI";
        case static_cast<uint8_t>(ItemId::MAGIC_HAT):          return "SOM";
        
        // Escudos
        case static_cast<uint8_t>(ItemId::TURTLE_SHIELD):      return "EST";
        case static_cast<uint8_t>(ItemId::IRON_SHIELD):        return "ESI";
        case static_cast<uint8_t>(ItemId::BOCA_SHIELD):        return "ESB";
        
        // Pociones
        case static_cast<uint8_t>(ItemId::HEALTH_POTION):      return "PVI";
        case static_cast<uint8_t>(ItemId::MANA_POTION):        return "PMA";
        
        // Oro
        case static_cast<uint8_t>(ItemId::GOLD_PILE):          return "ORO";
        
        default: return "???";
    }
}

const char* InventoryPanel::item_kind(uint8_t id) {
    if (id == 0) return nullptr;
    
    uint8_t item_enum = id;
    
    // Clasificar por tipo de item según Items.h
    // WEAPON_MELEE: 1-3
    if (item_enum >= static_cast<uint8_t>(ItemId::SWORD) &&
        item_enum <= static_cast<uint8_t>(ItemId::HAMMER))
        return "Arma Melee";

    // WEAPON_RANGED: 4-5
    if (item_enum >= static_cast<uint8_t>(ItemId::SIMPLE_BOW) &&
        item_enum <= static_cast<uint8_t>(ItemId::COMPOUND_BOW))
        return "Arma Ranged";

    // WEAPON_MAGIC: 6-9
    if (item_enum >= static_cast<uint8_t>(ItemId::ELVEN_FLUTE) &&
        item_enum <= static_cast<uint8_t>(ItemId::NUDOSO_STAFF))
        return "Arma Mágica";
    
    // ARMOR: 19-26
    if (item_enum >= static_cast<uint8_t>(ItemId::LEATHER_ARMOR) && 
        item_enum <= static_cast<uint8_t>(ItemId::PALADIN_ROYAL_ARMOR))
        return "Armadura";
    
    // HELMET: 27-29
    if (item_enum >= static_cast<uint8_t>(ItemId::HOOD) && 
        item_enum <= static_cast<uint8_t>(ItemId::MAGIC_HAT))
        return "Casco";
    
    // SHIELD: 30-32
    if (item_enum >= static_cast<uint8_t>(ItemId::TURTLE_SHIELD) && 
        item_enum <= static_cast<uint8_t>(ItemId::BOCA_SHIELD))
        return "Escudo";
    
    // POTION: 33-34
    if (item_enum >= static_cast<uint8_t>(ItemId::HEALTH_POTION) && 
        item_enum <= static_cast<uint8_t>(ItemId::MANA_POTION))
        return "Poción";
    
    // GOLD: 35
    if (item_enum == static_cast<uint8_t>(ItemId::GOLD_PILE))
        return "Oro";
    
    return nullptr;
}

//  Constructor / destructor

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

    // Fuente más pequeña para abreviaturas dentro del slot
    _font_sm = TTF_OpenFont(font_path.c_str(), std::max(8, font_size - 2));

    std::memset(_inventory, 0, sizeof(_inventory));
}

InventoryPanel::~InventoryPanel() {
    if (_font)    TTF_CloseFont(_font);
    if (_font_sm) TTF_CloseFont(_font_sm);
}

//  update

void InventoryPanel::update(const uint8_t inventory[INV_SIZE],
                             uint8_t equipped_wpn, uint8_t equipped_arm,
                             uint8_t equipped_helm, uint8_t equipped_shld) {
    std::memcpy(_inventory, inventory, INV_SIZE);
    _eq_wpn  = equipped_wpn;
    _eq_arm  = equipped_arm;
    _eq_helm = equipped_helm;
    _eq_shld = equipped_shld;
}

void InventoryPanel::register_item_texture(uint8_t item_id, SDL_Texture* tex) {
    _item_textures[item_id] = tex;
}

//  handle_event

bool InventoryPanel::handle_event(const SDL_Event& e, Queue<Command>* cmd_queue) {
    if (!_visible) return false;

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        _visible = false;
        return true;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;

        auto in = [](int mx, int my, const SDL_Rect& r) {
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
                _selected_slot = (_selected_slot == i) ? -1 : i;
                return true;
            }
        }

        if (in(mx, my, _panel_rect)) return true;
    }
    return false;
}

//  Helpers de dibujo

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

// Fondo de panel con doble borde estilo AO (borde exterior claro, interior oscuro)
void InventoryPanel::draw_panel_bg(int x, int y, int w, int h) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);

    // Sombra exterior
    SDL_SetRenderDrawColor(_renderer.Get(), 0, 0, 0, 180);
    SDL_Rect shadow{ x + 4, y + 4, w, h };
    SDL_RenderFillRect(_renderer.Get(), &shadow);

    // Fondo principal
    SDL_SetRenderDrawColor(_renderer.Get(), 22, 18, 14, 248);
    SDL_Rect bg{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &bg);

    // Borde exterior claro (estilo bisel)
    SDL_SetRenderDrawColor(_renderer.Get(), 80, 65, 45, 255);
    SDL_RenderDrawRect(_renderer.Get(), &bg);

    // Borde interior más oscuro (1px hacia adentro)
    SDL_SetRenderDrawColor(_renderer.Get(), 40, 32, 22, 255);
    SDL_Rect inner{ x + 1, y + 1, w - 2, h - 2 };
    SDL_RenderDrawRect(_renderer.Get(), &inner);
}

// Ícono de candado dibujado con primitivas SDL (para slots bloqueados futuros)
void InventoryPanel::draw_lock_icon(int cx, int cy, int sz) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_Color lc{ 110, 90, 55, 200 };
    SDL_SetRenderDrawColor(_renderer.Get(), lc.r, lc.g, lc.b, lc.a);

    // Cuerpo del candado (rectángulo inferior)
    int bw = sz * 6 / 10, bh = sz * 5 / 10;
    int bx = cx - bw / 2, by = cy;
    SDL_Rect body{ bx, by, bw, bh };
    SDL_RenderFillRect(_renderer.Get(), &body);

    // Arco superior (aproximación con líneas horizontales)
    int aw = sz * 4 / 10;
    int ah = sz * 4 / 10;
    int ay = cy - ah;
    for (int dy = 0; dy <= ah; dy++) {
        float t = static_cast<float>(dy) / ah;
        float rx = aw / 2.0f * std::sqrt(std::max(0.0f, 1.0f - t * t));
        if (rx < 1.5f) continue;
        // solo el arco exterior (borde)
        SDL_RenderDrawLine(_renderer.Get(),
            cx - static_cast<int>(rx), ay + dy,
            cx - static_cast<int>(rx) + 1, ay + dy);
        SDL_RenderDrawLine(_renderer.Get(),
            cx + static_cast<int>(rx) - 1, ay + dy,
            cx + static_cast<int>(rx), ay + dy);
    }

    // Ojo del candado
    SDL_SetRenderDrawColor(_renderer.Get(), 22, 18, 14, 255);
    SDL_Rect eye{ cx - 1, by + bh / 3, 3, bh / 2 };
    SDL_RenderFillRect(_renderer.Get(), &eye);
}

// Slot individual de la grilla
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

    // Degradado sutil: franja superior más clara
    SDL_SetRenderDrawColor(_renderer.Get(), 45, 38, 28, 60);
    SDL_Rect top_stripe{ x, y, size, size / 3 };
    SDL_RenderFillRect(_renderer.Get(), &top_stripe);

    // Borde exterior oscuro
    SDL_SetRenderDrawColor(_renderer.Get(), 10, 8, 5, 255);
    SDL_RenderDrawRect(_renderer.Get(), &slot_r);

    // Borde interior superior/izquierdo claro (highlight)
    SDL_Color hi_color = is_selected ? SDL_Color{200, 160, 40, 200}
                       : is_equipped ? SDL_Color{60, 160, 60, 180}
                       :               SDL_Color{55, 45, 32, 180};
    SDL_SetRenderDrawColor(_renderer.Get(), hi_color.r, hi_color.g, hi_color.b, hi_color.a);
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+1, x+size-2, y+1);   // top
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+1, x+1, y+size-2);   // left

    // Borde interior inferior/derecho oscuro (sombra)
    SDL_SetRenderDrawColor(_renderer.Get(), 8, 6, 4, 200);
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+size-2, x+size-2, y+size-2); // bottom
    SDL_RenderDrawLine(_renderer.Get(), x+size-2, y+1, x+size-2, y+size-2); // right

    if (is_empty) {
        return;
    }

    int cx = x + size / 2;
    int cy = y + size / 2;

    auto it = _item_textures.find(item);
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

// Fila de equip (sección superior del panel)
void InventoryPanel::draw_equip_row(int x, int y, int w, int h,
                                     const char* label, uint8_t slot_idx) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);

    bool has_item = (slot_idx != 0xFF && slot_idx < INV_SIZE && _inventory[slot_idx] != 0);
    uint8_t item_id = has_item ? _inventory[slot_idx] : 0;

    // Fondo
    if (has_item) {
        SDL_SetRenderDrawColor(_renderer.Get(), 16, 45, 16, 240);
    } else {
        SDL_SetRenderDrawColor(_renderer.Get(), 20, 16, 10, 230);
    }
    SDL_Rect r{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &r);

    // Borde
    SDL_Color border_c = has_item ? SDL_Color{55, 130, 55, 200}
                                  : SDL_Color{45, 38, 28, 180};
    SDL_SetRenderDrawColor(_renderer.Get(), border_c.r, border_c.g, border_c.b, border_c.a);
    SDL_RenderDrawRect(_renderer.Get(), &r);

    // Highlight superior
    SDL_SetRenderDrawColor(_renderer.Get(), 60, 50, 35, 100);
    SDL_RenderDrawLine(_renderer.Get(), x+1, y+1, x+w-2, y+1);

    // Etiqueta de slot (e.g. "Arma")
    SDL_Color lbl_c{ 120, 105, 75, 255 };
    draw_text(label, x + 6, y + (h - _font_size) / 2, lbl_c);

    // Nombre del item
    const char* name = has_item ? item_name(item_id) : nullptr;
    SDL_Color name_c = has_item ? SDL_Color{160, 230, 160, 255}
                                : SDL_Color{ 55,  50,  40, 255};
    std::string display = name ? std::string(name) : std::string("—");

    int lbl_w = 0, dummy = 0;
    if (_font) TTF_SizeUTF8(_font, label, &lbl_w, &dummy);
    draw_text(display, x + 6 + lbl_w + 10, y + (h - _font_size) / 2, name_c);
}

//  render

void InventoryPanel::render(int screen_w, int screen_h) {
    if (!_visible) return;

    const int ROWS        = (INV_SIZE + COLS - 1) / COLS;   // = 4 para 16 slots con 5 cols → 3.2 → 4
    const int GRID_W      = COLS * SLOT_SZ + (COLS - 1) * SLOT_GAP;
    const int PAD         = 14;
    const int TITLE_H     = _font_size + 10;
    const int EQ_ROW_H    = 24;
    const int EQ_SECTION  = 4 * (EQ_ROW_H + 3) + 8;   // 4 slots equipados
    const int GRID_H      = ROWS * SLOT_SZ + (ROWS - 1) * SLOT_GAP;
    const int LABEL_H     = _font_size + 6;
    const int ACTION_H    = 36;
    const int HINT_H      = _font_size + 8;

    const int MOD_W = GRID_W + PAD * 2;
    const int MOD_H = TITLE_H + EQ_SECTION + LABEL_H + GRID_H + ACTION_H + HINT_H + PAD * 2;

    const int mx0 = (screen_w - MOD_W) / 2;
    const int my0 = (screen_h - MOD_H) / 2;

    _panel_rect = { mx0, my0, MOD_W, MOD_H };

    // Fondo del panel
    draw_panel_bg(mx0, my0, MOD_W, MOD_H);

    SDL_Color gold    { 210, 175,  55, 255 };
    SDL_Color section { 150, 125,  75, 255 };
    SDL_Color white   { 230, 220, 200, 255 };
    SDL_Color dimgray { 90,  85,  70, 255  };

    int cy = my0 + PAD;

    // Título
    draw_text("INVENTARIO", mx0 + PAD, cy, gold);

    // Botón cerrar [X]
    _close_btn = { mx0 + MOD_W - 22, my0 + 6, 16, 16 };
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 120, 25, 25, 230);
    SDL_RenderFillRect(_renderer.Get(), &_close_btn);
    SDL_SetRenderDrawColor(_renderer.Get(), 190, 55, 55, 255);
    SDL_RenderDrawRect(_renderer.Get(), &_close_btn);
    draw_text_centered("X", _close_btn.x + 8, _close_btn.y + 8, white, _font_sm);

    // Separador
    cy += TITLE_H;
    SDL_SetRenderDrawColor(_renderer.Get(), 70, 58, 38, 180);
    SDL_RenderDrawLine(_renderer.Get(), mx0 + PAD, cy - 2, mx0 + MOD_W - PAD, cy - 2);

    // Sección equipamiento
    draw_text("Equipado", mx0 + PAD, cy, section);
    cy += _font_size + 4;

    struct EqEntry { const char* label; uint8_t slot_idx; };
    const EqEntry eq_entries[] = {
        { "Arma",     _eq_wpn  },
        { "Armadura", _eq_arm  },
        { "Casco",    _eq_helm },
        { "Escudo",   _eq_shld },
    };
    for (const auto& eq : eq_entries) {
        draw_equip_row(mx0 + PAD, cy, GRID_W, EQ_ROW_H, eq.label, eq.slot_idx);
        cy += EQ_ROW_H + 3;
    }
    cy += 8;

    // Separador
    SDL_SetRenderDrawColor(_renderer.Get(), 70, 58, 38, 180);
    SDL_RenderDrawLine(_renderer.Get(), mx0 + PAD, cy - 2, mx0 + MOD_W - PAD, cy - 2);

    // Sección mochila
    draw_text("Mochila", mx0 + PAD, cy, section);
    cy += LABEL_H;

    const int gx0 = mx0 + PAD;

    for (int i = 0; i < INV_SIZE; i++) {
        int col = i % COLS;
        int row = i / COLS;
        int sx  = gx0 + col * (SLOT_SZ + SLOT_GAP);
        int sy  = cy  + row * (SLOT_SZ + SLOT_GAP);
        draw_slot(sx, sy, SLOT_SZ, i);
    }

    cy += GRID_H + 8;

    // Tooltip del slot seleccionado
    if (_selected_slot >= 0 && _selected_slot < INV_SIZE
        && _inventory[_selected_slot] != 0) {

        uint8_t item_id = _inventory[_selected_slot];
        const char* name = item_name(item_id);
        const char* kind = item_kind(item_id);

        // Nombre e item type
        if (name) draw_text(name, mx0 + PAD, cy, white);
        if (kind) {
            int nw = 0, nh = 0;
            if (_font && name) TTF_SizeUTF8(_font, name, &nw, &nh);
            draw_text(std::string(" (") + kind + ")", mx0 + PAD + nw, cy, dimgray);
        }
        cy += _font_size + 4;

        const int BTN_W = (GRID_W - 6) / 2;
        const int BTN_H = 22;

        // Botón usar / desequipar / equipar
        bool is_potion   = (item_id == 40 || item_id == 41);
        
        bool is_equipped = (_selected_slot == _eq_wpn  || _selected_slot == _eq_arm ||
                            _selected_slot == _eq_helm || _selected_slot == _eq_shld);

        const char* btn_label = is_potion    ? "Usar"
                              : is_equipped  ? "Desequipar"
                                             : "Equipar";

        _equip_btn = { mx0 + PAD, cy, BTN_W, BTN_H };
        if (is_equipped) {
            // Estilo visual del botón Desequipar (marrón/ámbar)
            SDL_SetRenderDrawColor(_renderer.Get(), 70, 50, 18, 230);
            SDL_RenderFillRect(_renderer.Get(), &_equip_btn);
            SDL_SetRenderDrawColor(_renderer.Get(), 170, 130, 50, 200);
            SDL_RenderDrawRect(_renderer.Get(), &_equip_btn);
            draw_text_centered(btn_label,
                               _equip_btn.x + BTN_W / 2, _equip_btn.y + BTN_H / 2,
                               SDL_Color{235, 200, 110, 255}, _font_sm);
        } else {
            // Estilo visual del botón Equipar/Usar (verde)
            SDL_SetRenderDrawColor(_renderer.Get(), 25, 70, 25, 230);
            SDL_RenderFillRect(_renderer.Get(), &_equip_btn);
            SDL_SetRenderDrawColor(_renderer.Get(), 65, 155, 65, 200);
            SDL_RenderDrawRect(_renderer.Get(), &_equip_btn);
            draw_text_centered(btn_label,
                               _equip_btn.x + BTN_W / 2, _equip_btn.y + BTN_H / 2,
                               SDL_Color{160, 235, 160, 255}, _font_sm);
        }

        // Botón tirar
        _drop_btn = { mx0 + PAD + BTN_W + 6, cy, BTN_W, BTN_H };
        SDL_SetRenderDrawColor(_renderer.Get(), 70, 18, 18, 230);
        SDL_RenderFillRect(_renderer.Get(), &_drop_btn);
        SDL_SetRenderDrawColor(_renderer.Get(), 155, 55, 55, 200);
        SDL_RenderDrawRect(_renderer.Get(), &_drop_btn);
        draw_text_centered("Tirar",
                           _drop_btn.x + BTN_W / 2, _drop_btn.y + BTN_H / 2,
                           SDL_Color{235, 150, 150, 255}, _font_sm);

    } else {
        _equip_btn = {};
        _drop_btn  = {};
        draw_text("Selecciona un objeto para ver opciones", mx0 + PAD, cy, dimgray);
        cy += _font_size + 4;
    }

    // Hint
    draw_text("ESC para cerrar | Click para seleccionar",
              mx0 + PAD, my0 + MOD_H - HINT_H, dimgray);
}
