#include "StatsPanel.h"
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
                        uint32_t gold, uint8_t level, bool meditating, bool is_ghost) {
    _hp        = hp;
    _max_hp    = std::max<uint16_t>(1, max_hp);
    _mp        = mp;
    _max_mp    = max_mp;  // 0 = guerrero, la sección se oculta
    _gold      = gold;
    _level     = level;
    _meditating = meditating;
    _is_ghost   = is_ghost;
}

bool StatsPanel::handle_event(const SDL_Event& e) {
    _inv_clicked = false;
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;
        if (mx >= _inv_btn_rect.x && mx < _inv_btn_rect.x + _inv_btn_rect.w &&
            my >= _inv_btn_rect.y && my < _inv_btn_rect.y + _inv_btn_rect.h) {
            _inv_clicked = true;
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

    // Fondo
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), bg.r, bg.g, bg.b, bg.a);
    SDL_Rect bg_rect{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &bg_rect);

    // Relleno
    int filled_w = static_cast<int>(w * fraction);
    if (filled_w > 0) {
        SDL_SetRenderDrawColor(_renderer.Get(), fill.r, fill.g, fill.b, fill.a);
        SDL_Rect fill_rect{ x, y, filled_w, h };
        SDL_RenderFillRect(_renderer.Get(), &fill_rect);
    }

    // Borde
    SDL_SetRenderDrawColor(_renderer.Get(), 80, 80, 80, 200);
    SDL_RenderDrawRect(_renderer.Get(), &bg_rect);
}

void StatsPanel::draw_rounded_rect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), color.r, color.g, color.b, color.a);
    SDL_Rect r{ x, y, w, h };
    SDL_RenderFillRect(_renderer.Get(), &r);
    // Borde más claro
    SDL_SetRenderDrawColor(_renderer.Get(),
        std::min(255, color.r + 50),
        std::min(255, color.g + 50),
        std::min(255, color.b + 50), 220);
    SDL_RenderDrawRect(_renderer.Get(), &r);
}

void StatsPanel::render(int screen_w, int screen_h) {
    const int PW  = PANEL_W;
    const int px  = screen_w - PW;
    const int pad = 10;
    const int lh  = _font_size + 6;

    // Fondo semitransparente
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 15, 10, 5, 200);
    SDL_Rect bg{ px, 0, PW, screen_h };
    SDL_RenderFillRect(_renderer.Get(), &bg);

    // Línea separadora izquierda
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 70, 30, 255);
    SDL_RenderDrawLine(_renderer.Get(), px, 0, px, screen_h);

    int cy = pad;
    SDL_Color white  { 255, 255, 255, 255 };
    SDL_Color gold_c { 255, 210,  50, 255 };
    SDL_Color ghost_c{ 180, 200, 255, 255 };

    // Nivel
    std::string lvl_str = "Nivel  " + std::to_string(_level);
    if (_is_ghost)   lvl_str += "  [FANTASMA]";
    if (_meditating) lvl_str += "  [meditando]";
    SDL_Color lvl_color = _is_ghost ? ghost_c : gold_c;
    draw_text(lvl_str, px + pad, cy, lvl_color);
    cy += lh + 4;

    // Vida
    draw_text("Vida", px + pad, cy, white);
    cy += lh - 2;

    float hp_frac = static_cast<float>(_hp) / _max_hp;
    SDL_Color hp_fill  = hp_frac > 0.5f
                         ? SDL_Color{  50, 200,  50, 255 }
                         : (hp_frac > 0.25f
                            ? SDL_Color{ 230, 180,  0, 255 }
                            : SDL_Color{ 210,  40, 40, 255 });
    SDL_Color hp_bg    { 40, 10, 10, 200 };
    draw_bar(px + pad, cy, PW - pad * 2, 18, hp_frac, hp_fill, hp_bg);

    // Texto centrado en la barra
    std::string hp_str = std::to_string(_hp) + " / " + std::to_string(_max_hp);
    SDL_Color bar_text { 240, 240, 240, 255 };
    draw_text(hp_str, px + pad + 4, cy + 1, bar_text);
    cy += 18 + 8;

    if (_max_mp > 0) {
        draw_text("Maná", px + pad, cy, white);
        cy += lh - 2;
        float mp_frac = static_cast<float>(_mp) / static_cast<float>(_max_mp);
        SDL_Color mp_fill { 50, 100, 220, 255 };
        SDL_Color mp_bg   { 10,  10,  40, 200 };
        draw_bar(px + pad, cy, PW - pad * 2, 18, mp_frac, mp_fill, mp_bg);
        std::string mp_str = std::to_string(_mp) + " / " + std::to_string(_max_mp);
        draw_text(mp_str, px + pad + 4, cy + 1, bar_text);
        cy += 18 + 12;
    }

    // Oro
    draw_text("Oro:  " + std::to_string(_gold), px + pad, cy, gold_c);
    cy += lh + 14;

    // Botón Inventario
    const int btn_h = 36;
    const int btn_w = PW - pad * 2;
    _inv_btn_rect = { px + pad, cy, btn_w, btn_h };

    SDL_Color btn_bg { 70, 50, 20, 230 };
    draw_rounded_rect(px + pad, cy, btn_w, btn_h, btn_bg);

    SDL_Color btn_text{ 255, 230, 150, 255 };
    draw_text("[ Inventario ]", px + pad + 20, cy + (btn_h - _font_size) / 2, btn_text);
}
