#include "PositionLabel.h"
#include <stdexcept>

PositionLabel::PositionLabel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size)
    : _renderer(renderer), _font_size(font_size) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0)
            throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    _font = TTF_OpenFont(font_path.c_str(), font_size);
    if (!_font)
        throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());
}

PositionLabel::~PositionLabel() {
    if (_font) TTF_CloseFont(_font);
}

void PositionLabel::toggle_visibility() { _visible = !_visible; }

void PositionLabel::update(int tile_x, int tile_y) {
    _tile_x = tile_x;
    _tile_y = tile_y;
}

void PositionLabel::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty() || !_font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(_font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
    SDL_Rect dst{ x, y, surf->w, surf->h };
    SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void PositionLabel::render(int /*screen_w*/, int /*screen_h*/) {
    if (!_visible) return;

    std::string text = "X = " + std::to_string(_tile_x) + "  Y = " + std::to_string(_tile_y);

    // Calcular dimensiones del texto primero para saber el tamaño del fondo
    int text_w = 0, text_h = 0;
    if (_font) {
        TTF_SizeUTF8(_font, text.c_str(), &text_w, &text_h);
    }
    if (text_w == 0) text_w = 120;
    if (text_h == 0) text_h = _font_size;

    const int padding = 6;
    const int box_w = text_w + padding * 2;
    const int box_h = text_h + padding * 2;
    const int x = 8;       // misma posición X que el chat
    const int y = 8 + 210; // debajo del chat (chat ocupa ~200px aprox)

    // Fondo rojo semitransparente
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 180, 0, 0, 200);
    SDL_Rect bg{ x, y, box_w, box_h };
    SDL_RenderFillRect(_renderer.Get(), &bg);

    // Texto blanco
    SDL_Color white{ 255, 255, 255, 255 };
    draw_text(text, x + padding, y + padding, white);
}