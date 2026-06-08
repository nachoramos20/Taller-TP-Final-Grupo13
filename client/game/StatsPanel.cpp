#include "StatsPanel.h"
#include "../../common/protocol/protocol.h"
#include <stdexcept>
#include <string>
#include <algorithm>

StatsPanel::StatsPanel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size)
    : _renderer(renderer), _font_size(font_size) {
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

void StatsPanel::update(uint16_t hp, uint16_t max_hp, uint16_t mp, uint16_t max_mp,
                        uint32_t gold, uint8_t level, bool meditating, bool is_ghost,
                        uint8_t cls, uint8_t equipped_weapon_item_id) {
    _hp = hp; _max_hp = std::max<uint16_t>(1, max_hp);
    _mp = mp; _max_mp = max_mp;
    _gold = gold; _level = level;
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
    switch (static_cast<Class>(cls)) {
        case Class::MAGE: return {
            { (uint8_t)SpellId::MAGIC_MISSILE, "Misil Mágico", 8  },
            { (uint8_t)SpellId::FIREBALL,      "Bola de Fuego",20 },
            { (uint8_t)SpellId::LIGHTNING,     "Rayo",         35 },
        };
        case Class::CLERIC: return {
            { (uint8_t)SpellId::DIVINE_SMITE,  "Castigo Div.", 10 },
            { (uint8_t)SpellId::HOLY_FLAME,    "Llama Sagrada",22 },
            { (uint8_t)SpellId::LIGHT_STORM,   "Torm. de Luz", 40 },
        };
        case Class::PALADIN: return {
            { (uint8_t)SpellId::SACRED_STRIKE, "Golpe Sagrado",6  },
            { (uint8_t)SpellId::FAITH_SPEAR,   "Lanza de Fe",  15 },
            { (uint8_t)SpellId::JUDGEMENT,     "Juicio",       30 },
        };
        default: return {};
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
    const int pad = 10;
    const int lh  = _font_size + 6;

    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 15, 10, 5, 200);
    SDL_Rect bg{ px, 0, PW, screen_h };
    SDL_RenderFillRect(_renderer.Get(), &bg);
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 70, 30, 255);
    SDL_RenderDrawLine(_renderer.Get(), px, 0, px, screen_h);

    int cy = pad;
    SDL_Color white  { 255, 255, 255, 255 };
    SDL_Color gold_c { 255, 210,  50, 255 };
    SDL_Color ghost_c{ 180, 200, 255, 255 };
    SDL_Color dim    { 130, 120, 100, 255 };

    std::string lvl_str = "Nivel  " + std::to_string(_level);
    if (_is_ghost)   lvl_str += "  [FANTASMA]";
    if (_meditating) lvl_str += "  [med]";
    draw_text(lvl_str, px + pad, cy, _is_ghost ? ghost_c : gold_c);
    cy += lh + 4;

    draw_text("Vida", px + pad, cy, white); cy += lh - 2;
    float hp_frac = static_cast<float>(_hp) / _max_hp;
    SDL_Color hp_fill = hp_frac > 0.5f ? SDL_Color{50,200,50,255}
                       : hp_frac > 0.25f ? SDL_Color{230,180,0,255}
                                         : SDL_Color{210,40,40,255};
    draw_bar(px + pad, cy, PW - pad*2, 18, hp_frac, hp_fill, {40,10,10,200});
    draw_text(std::to_string(_hp) + " / " + std::to_string(_max_hp),
              px + pad + 4, cy + 1, {240,240,240,255});
    cy += 18 + 8;

    if (_max_mp > 0) {
        draw_text("Maná", px + pad, cy, white); cy += lh - 2;
        float mp_frac = static_cast<float>(_mp) / static_cast<float>(_max_mp);
        draw_bar(px + pad, cy, PW - pad*2, 18, mp_frac, {50,100,220,255}, {10,10,40,200});
        draw_text(std::to_string(_mp) + " / " + std::to_string(_max_mp),
                  px + pad + 4, cy + 1, {240,240,240,255});
        cy += 18 + 12;
    }

    draw_text("Oro:  " + std::to_string(_gold), px + pad, cy, gold_c);
    cy += lh + 10;

    // Botón Inventario
    const int btn_h = 32;
    const int btn_w = PW - pad * 2;
    _inv_btn_rect = { px + pad, cy, btn_w, btn_h };
    draw_rounded_rect(px + pad, cy, btn_w, btn_h, {70, 50, 20, 230});
    draw_text("[ Inventario ]", px + pad + 20, cy + (btn_h - _font_size) / 2,
              {255, 230, 150, 255});
    cy += btn_h + 12;

    // Sección hechizos
    auto spells = spells_for_class(_cls);
    _spell_btn_count = 0;
    for (int i = 0; i < MAX_SPELL_BTNS; i++) _spell_btn_rect[i] = {};

    if (spells.empty()) {
        draw_text("Sin hechizos (Guerrero)", px + pad, cy, dim);
        return;
    }

    draw_text("Hechizos", px + pad, cy, {200, 170, 240, 255});
    cy += _font_size + 6;

    bool magic_weapon = weapon_enables_spells(_eq_weapon_item);
    if (!magic_weapon) {
        draw_text("Equipá un báculo/vara", px + pad, cy, dim);
        cy += _font_size + 6;
    }

    const int sb_h = 28;
    for (size_t i = 0; i < spells.size() && (int)i < MAX_SPELL_BTNS; i++) {
        const auto& s = spells[i];
        bool selected = _cast_mode && _selected_spell == s.id;
        bool enabled  = magic_weapon && _mp >= s.mana && !_is_ghost;

        SDL_Color col = !enabled ? SDL_Color{ 40, 30, 50, 220 }
                       : selected ? SDL_Color{ 110, 60, 160, 240 }
                                  : SDL_Color{ 55, 35, 90, 230 };
        draw_rounded_rect(px + pad, cy, btn_w, sb_h, col);

        SDL_Color txt = !enabled ? dim
                       : selected ? SDL_Color{ 255, 240, 200, 255 }
                                  : SDL_Color{ 220, 200, 255, 255 };
        std::string label = std::string(selected ? "* " : "  ") + s.label +
                            "  (" + std::to_string(s.mana) + " mp)";
        draw_text(label, px + pad + 6, cy + (sb_h - _font_size) / 2, txt);

        _spell_btn_rect[i] = { px + pad, cy, btn_w, sb_h };
        _spell_btn_id[i]   = s.id;
        _spell_btn_count++;
        cy += sb_h + 4;
    }

    if (_cast_mode) {
        draw_text("Modo hechizo: ON", px + pad, cy + 4, {200, 255, 200, 255});
    }
}
