#include "StatsPanel.h"
#include "../../common/protocol/protocol.h"
#include "../audio/GameAudioService.h"
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cmath>

StatsPanel::StatsPanel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size,
                       GameAudioService* audio)
    : _renderer(renderer), _audio(audio), _font_size(font_size) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0)
            throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    _font = TTF_OpenFont(font_path.c_str(), font_size);
    if (!_font)
        throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());
}

StatsPanel::~StatsPanel() {
    if (_font) TTF_CloseFont(_font);
}

void StatsPanel::set_inventory_ref(const uint8_t* inv, int size) {
    _inv_ref  = inv;
    _inv_size = size;
}

void StatsPanel::update(uint16_t hp, uint16_t max_hp, uint16_t mp, uint16_t max_mp,
                        uint32_t gold, uint8_t level, uint32_t exp, bool meditating, bool is_ghost,
                        uint8_t cls, uint8_t equipped_weapon_item_id) {
    _hp = hp; _max_hp = std::max<uint16_t>(1, max_hp);
    _mp = mp; _max_mp = max_mp;
    _gold = gold; _level = level; _exp = exp;
    _meditating = meditating; _is_ghost = is_ghost;
    _cls = cls;
    _eq_weapon_item = equipped_weapon_item_id;

    // Si se desequipa el arma mágica, apagar modo hechizo
    if (_cast_mode && !weapon_enables_spells(_eq_weapon_item)) {
        _cast_mode = false;
        _selected_spell = 0;
    }
}

std::vector<StatsPanel::SpellInfo> StatsPanel::spells_for_class(uint8_t cls) const {
    // BUG FIX anim: rangos espejados del config/spells.toml del servidor.
    // Formato: { spell_id, label, mana_cost, range_tiles }
    switch (static_cast<Class>(cls)) {
        case Class::MAGE: return {
            { (uint8_t)SpellId::BURST,                     "Explosión",                     9,  8 },
            { (uint8_t)SpellId::POISON_AREA,               "Area de veneno",               18,  6 },
            { (uint8_t)SpellId::SKULL_EXPLOSION,           "Explosión calavérica",         32,  6 },
        };
        case Class::CLERIC: return {
            { (uint8_t)SpellId::ICE_ORB,                   "Orbe de hielo",                 8,  5 },
            { (uint8_t)SpellId::GRAVITATIONAL_TORNAD,      "Tornado gravitatorio",         22,  5 },
            { (uint8_t)SpellId::THUNDERSTORM,              "Tormenta eléctrica",           38,  6 },
        };
        case Class::PALADIN: return {
            { (uint8_t)SpellId::ORB_OF_EMPTINESS,          "Orbe de vacío",                10,  4 },
            { (uint8_t)SpellId::VACUUM_GAP,                "Brecha de vacío",              22,  4 },
            { (uint8_t)SpellId::TORNADO_OF_DARKNESS,       "Tornado de oscuridad",         40,  2 },
        };
        default: return {};
    }
}

uint16_t StatsPanel::selected_spell_mana_cost() const {
    if (!_cast_mode || _selected_spell == 0) return 0;
    for (const auto& s : spells_for_class(_cls))
        if (s.id == _selected_spell) return s.mana;
    return 0;
}

int StatsPanel::selected_spell_range() const {
    if (!_cast_mode || _selected_spell == 0) return 0;
    for (const auto& s : spells_for_class(_cls))
        if (s.id == _selected_spell) return s.range;
    return 0;
}

void StatsPanel::activate_spell_by_index(int index) {
    auto spells = spells_for_class(_cls);
    if (index < 0 || index >= (int)spells.size()) return;
    const auto& s = spells[index];
    if (_audio) _audio->click();
    if (_cast_mode && _selected_spell == s.id) {
        _cast_mode = false;
        _selected_spell = 0;
    } else {
        _cast_mode = true;
        _selected_spell = s.id;
    }
}

bool StatsPanel::handle_event(const SDL_Event& e) {
    _inv_clicked = false;
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT)
        return false;

    int mx = e.button.x, my = e.button.y;
    auto in = [&](const SDL_Rect& r) {
        return r.w > 0 && mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
    };

    if (in(_inv_btn_rect)) { _inv_clicked = true; return true; }

    for (int i = 0; i < _spell_btn_count; i++) {
        if (in(_spell_btn_rect[i])) {
            uint8_t id = _spell_btn_id[i];
            if (_audio) _audio->click();
            if (_cast_mode && _selected_spell == id) {
                _cast_mode = false;
                _selected_spell = 0;
            } else {
                _cast_mode = true;
                _selected_spell = id;
            }
            return true;
        }
    }
    return false;
}

void StatsPanel::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty() || !_font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(_font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
    SDL_Rect dst{ x, y, surf->w, surf->h };
    SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void StatsPanel::draw_bar(int x, int y, int w, int h,
                          float fraction, SDL_Color fill, SDL_Color bg) {
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), bg.r, bg.g, bg.b, bg.a);
    SDL_Rect bg_rect{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &bg_rect);
    int filled_w = static_cast<int>(w * fraction);
    if (filled_w > 0) {
        SDL_SetRenderDrawColor(_renderer.Get(), fill.r, fill.g, fill.b, fill.a);
        SDL_Rect fill_rect{ x, y, filled_w, h };
        SDL_RenderFillRect(_renderer.Get(), &fill_rect);
    }
    SDL_SetRenderDrawColor(_renderer.Get(), 80, 80, 80, 200);
    SDL_RenderDrawRect(_renderer.Get(), &bg_rect);
}

void StatsPanel::draw_rounded_rect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), color.r, color.g, color.b, color.a);
    SDL_Rect r{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &r);
    SDL_SetRenderDrawColor(_renderer.Get(),
        std::min(255, color.r + 50), std::min(255, color.g + 50),
        std::min(255, color.b + 50), 220);
    SDL_RenderDrawRect(_renderer.Get(), &r);
}

void StatsPanel::render(int screen_w, int screen_h) {
    const int PW  = PANEL_W;
    const int px  = screen_w - PW;
    const int pad = 12;
    const int lh  = _font_size + 6;

    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 12, 8, 4, 220);
    SDL_Rect bg{ px, 0, PW, screen_h };
    SDL_RenderFillRect(_renderer.Get(), &bg);

    // Borde superior del panel
    SDL_SetRenderDrawColor(_renderer.Get(), 180, 140, 60, 255);
    SDL_RenderDrawLine(_renderer.Get(), px, 0, px + PW, 0);
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 70, 30, 255);
    SDL_RenderDrawLine(_renderer.Get(), px, 1, px + PW, 1);

    int cy = pad + 4;
    SDL_Color white    { 245, 245, 220, 255 };
    SDL_Color gold_c   { 255, 200, 50, 255 };
    SDL_Color ghost_c  { 180, 200, 255, 255 };
    SDL_Color dim      { 120, 110, 90, 255 };

    auto draw_text_right = [&](const std::string& text, int right_x, int y, SDL_Color color) {
        if (text.empty() || !_font) return;
        int text_w = 0, text_h = 0;
        TTF_SizeUTF8(_font, text.c_str(), &text_w, &text_h);
        draw_text(text, right_x - text_w, y, color);
    };

    // ─── Header: Username + Nivel ───
    if (!_username.empty()) {
        SDL_Color name_col = _is_ghost ? ghost_c : SDL_Color{255, 220, 120, 255};
        draw_text(_username, px + pad, cy, name_col);
        cy += lh - 2;
    }

    std::string lvl_str = "Nivel " + std::to_string(_level);
    draw_text(lvl_str, px + pad, cy, _is_ghost ? ghost_c : gold_c);

    // Badge de estado a la derecha
    if (_is_ghost || _meditating) {
        std::string badge = _is_ghost ? "[FANTASMA]" : "[MEDITANDO]";
        SDL_Color badge_col = _is_ghost ? ghost_c : SDL_Color{100, 200, 100, 255};
        draw_text_right(badge, px + PW - pad, cy, badge_col);
    }
    cy += lh + 8;

    // Divisor visual
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
    SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
    cy += 6;

    // ─── HP ───
    draw_text("Vida", px + pad, cy, white);
    std::string hp_str = std::to_string(_hp) + " / " + std::to_string(_max_hp);
    draw_text_right(hp_str, px + PW - pad, cy, {220, 220, 220, 255});
    cy += lh - 2;

    float hp_frac = static_cast<float>(_hp) / _max_hp;
    SDL_Color hp_fill = hp_frac > 0.5f ? SDL_Color{80, 220, 80, 255}
                       : hp_frac > 0.25f ? SDL_Color{255, 180, 0, 255}
                                         : SDL_Color{220, 50, 50, 255};
    draw_bar(px + pad, cy, PW - pad*2, 20, hp_frac, hp_fill, {30, 10, 10, 180});
    cy += 20 + 10;

    // ─── Maná (si aplica) ───
    if (_max_mp > 0) {
        draw_text("Maná", px + pad, cy, white);
        std::string mp_str = std::to_string(_mp) + " / " + std::to_string(_max_mp);
        draw_text_right(mp_str, px + PW - pad, cy, {200, 200, 220, 255});
        cy += lh - 2;

        float mp_frac = static_cast<float>(_mp) / static_cast<float>(_max_mp);
        draw_bar(px + pad, cy, PW - pad*2, 20, mp_frac, {70, 130, 220, 255}, {15, 15, 50, 180});
        cy += 20 + 10;
    }

    // ─── EXP ───
    draw_text("Experiencia", px + pad, cy, white);

    uint32_t exp_current_level_floor = (_level <= 1) ? 0
        : static_cast<uint32_t>(1000.0 * std::pow(static_cast<double>(_level - 1), 1.8));

    uint32_t exp_next_level_ceil = (_level >= 50) ? 0xFFFFFFFF
        : static_cast<uint32_t>(1000.0 * std::pow(static_cast<double>(_level), 1.8));

    uint32_t exp_needed_for_this_level = exp_next_level_ceil - exp_current_level_floor;

    uint32_t exp_progress_in_level = 0;
    if (_exp > exp_current_level_floor)
        exp_progress_in_level = _exp - exp_current_level_floor;

    if (exp_progress_in_level > exp_needed_for_this_level)
        exp_progress_in_level = exp_needed_for_this_level;

    std::string exp_str;
    if (_level >= 50) {
        exp_str = "MAX";
    } else {
        exp_str = std::to_string(exp_progress_in_level) + " / " + std::to_string(exp_needed_for_this_level);
    }

    draw_text_right(exp_str, px + PW - pad, cy, {200, 150, 220, 255});
    cy += lh - 2;

    float exp_frac = (_level >= 50) ? 1.0f : static_cast<float>(exp_progress_in_level) / exp_needed_for_this_level;
    draw_bar(px + pad, cy, PW - pad*2, 16, exp_frac, {170, 120, 200, 255}, {40, 25, 60, 180});
    cy += 16 + 10;

    // ─── Oro ───
    draw_text("Oro", px + pad, cy, white);
    draw_text_right(std::to_string(_gold), px + PW - pad, cy, gold_c);
    cy += lh + 10;

    // Divisor visual
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
    SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
    cy += 8;

    // ─── Botón Inventario ───
    const int btn_h = 36;
    const int btn_w = PW - pad * 2;
    _inv_btn_rect = { px + pad, cy, btn_w, btn_h };
    draw_rounded_rect(px + pad, cy, btn_w, btn_h, {100, 70, 30, 240});
    draw_text("Inventario  [Tab]", px + pad + (btn_w - 100) / 2, cy + (btn_h - _font_size) / 2,
              {255, 240, 160, 255});
    cy += btn_h + 10;

    // ─── Sección Hechizos ───
    auto spells = spells_for_class(_cls);
    _spell_btn_count = 0;
    for (int i = 0; i < MAX_SPELL_BTNS; i++) _spell_btn_rect[i] = {};

    // Divisor visual
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
    SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
    cy += 8;

    if (spells.empty()) {
        draw_text("Sin hechizos (Guerrero)", px + pad, cy, dim);
        cy += lh + 4;

        // ─── Sección Atajos (guerrero) ───
        SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
        SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
        cy += 8;
        draw_text("Atajos de teclado", px + pad, cy, {200, 170, 240, 255});
        cy += lh + 4;

        const char* shortcuts[][2] = {
            { "Tab", "Inventario" },
            { "M", "Meditar" },
            { "R", "Resucitar" },
            { "E", "Recoger item" },
            { "Q", "Tirar item" },
            { "P", "Usar poción" },
        };
        for (auto& sc : shortcuts) {
            // Badge de tecla
            std::string key_label = sc[0];
            std::string action_label = sc[1];
            int key_w = 0, key_h = 0;
            TTF_SizeUTF8(_font, key_label.c_str(), &key_w, &key_h);
            int badge_w = key_w + 8;
            draw_rounded_rect(px + pad, cy, badge_w, lh, {80, 60, 30, 220});
            draw_text(key_label, px + pad + 4, cy + 1, {255, 240, 160, 255});
            draw_text(action_label, px + pad + badge_w + 6, cy + 1, dim);
            cy += lh + 3;
        }
        return;
    }

    draw_text("Hechizos", px + pad, cy, {200, 170, 240, 255});
    cy += lh + 4;

    bool magic_weapon = weapon_enables_spells(_eq_weapon_item);
    if (!magic_weapon) {
        draw_text("Equipá báculo/vara", px + pad, cy, dim);
        cy += lh + 6;
    }

    // hechizos en dos líneas por botón 
    const int sb_h = 8 + _font_size + 4 + _font_size + 8;
    const int sb_w = PW - pad * 2;
    for (size_t i = 0; i < spells.size() && (int)i < MAX_SPELL_BTNS; i++) {
        const auto& s = spells[i];
        bool selected = _cast_mode && _selected_spell == s.id;
        bool enabled  = magic_weapon && _mp >= s.mana && !_is_ghost;

        SDL_Color col = !enabled ? SDL_Color{ 50, 40, 60, 220 }
                       : selected ? SDL_Color{ 130, 80, 180, 250 }
                                  : SDL_Color{ 70, 50, 100, 230 };
        draw_rounded_rect(px + pad, cy, sb_w, sb_h, col);

        SDL_Color txt = !enabled ? dim
                       : selected ? SDL_Color{ 255, 245, 200, 255 }
                                  : SDL_Color{ 220, 200, 255, 255 };

        // Nombre del hechizo
        std::string label = selected ? "* " : "  ";
        label += s.label;
        draw_text(label, px + pad + 8, cy + 8, txt);

        // Costo + atajo
        std::string shortcut_str = std::string("[") + std::to_string(i + 1) + "]";
        std::string cost_str = std::to_string(s.mana) + "mp " + shortcut_str;
        draw_text(cost_str, px + pad + 8, cy + 8 + _font_size + 4, dim);

        _spell_btn_rect[i] = { px + pad, cy, sb_w, sb_h };
        _spell_btn_id[i]   = s.id;
        _spell_btn_count++;
        cy += sb_h + 6;
    }

    if (_cast_mode) {
        SDL_SetRenderDrawColor(_renderer.Get(), 100, 200, 100, 150);
        SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
        cy += 6;
        draw_text("Modo hechizo ACTIVO", px + pad, cy, {150, 255, 150, 255});
        cy += lh + 4;
    } else {
        cy += 4;
    }

    // ─── Sección Atajos generales ───
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
    SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
    cy += 8;
    draw_text("Atajos de teclado", px + pad, cy, {200, 170, 240, 255});
    cy += lh + 4;

    const char* shortcuts[][2] = {
        { "Tab", "Inventario" },
        { "M", "Meditar" },
        { "R", "Resucitar" },
        { "E", "Recoger item" },
        { "Q", "Tirar item" },
        { "P", "Usar poción" },
    };
    for (auto& sc : shortcuts) {
        std::string key_label = sc[0];
        std::string action_label = sc[1];
        int key_w = 0, key_h = 0;
        TTF_SizeUTF8(_font, key_label.c_str(), &key_w, &key_h);
        int badge_w = key_w + 8;
        draw_rounded_rect(px + pad, cy, badge_w, lh, {80, 60, 30, 220});
        draw_text(key_label, px + pad + 4, cy + 1, {255, 240, 160, 255});
        draw_text(action_label, px + pad + badge_w + 6, cy + 1, dim);
        cy += lh + 3;
    }
}
