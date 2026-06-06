#include "ChatWidget.h"
#include <stdexcept>

ChatWidget::ChatWidget(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size)
    : _renderer(renderer), _font_size(font_size) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0)
            throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    _font = TTF_OpenFont(font_path.c_str(), font_size);
    if (!_font)
        throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());
}

ChatWidget::~ChatWidget() {
    if (_font) TTF_CloseFont(_font);
}

bool ChatWidget::handle_event(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
        if (!_input_active) {
            _input_active = true;
            _input_buffer.clear();
            SDL_StartTextInput();
            return true;
        } else {
            if (!_input_buffer.empty() && _on_submit) _on_submit(_input_buffer);
            _input_buffer.clear();
            _input_active = false;
            SDL_StopTextInput();
            return true;
        }
    }
    if (!_input_active) return false;

    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            _input_active = false;
            _input_buffer.clear();
            SDL_StopTextInput();
            return true;
        }
        if (e.key.keysym.sym == SDLK_BACKSPACE && !_input_buffer.empty()) {
            _input_buffer.pop_back();
            return true;
        }
        return true; // consumo todas las teclas para que no muevan al player
    }
    if (e.type == SDL_TEXTINPUT) {
        _input_buffer += e.text.text;
        return true;
    }
    return false;
}

void ChatWidget::add_message(const std::string& text) {
    _messages.push_back(text);
    while (_messages.size() > 8) _messages.pop_front();
}

void ChatWidget::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty() || !_font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(_font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
    SDL_Rect dst{ x, y, surf->w, surf->h };
    SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void ChatWidget::render(int screen_w, int /*screen_h*/) {
    const int chat_w = 480;
    const int line_h = _font_size + 4;
    const int box_h  = line_h * (int)_messages.size() + (_input_active ? line_h + 6 : 0) + 8;
    const int x = 8;
    const int y = 8;

    if (box_h > 8) {
        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer.Get(), 0, 0, 0, 140);
        SDL_Rect bg{ x - 4, y - 4, chat_w, box_h };
        SDL_RenderFillRect(_renderer.Get(), &bg);
    }

    SDL_Color white{ 255, 255, 255, 255 };
    int cy = y;
    for (const auto& m : _messages) {
        draw_text(m, x, cy, white);
        cy += line_h;
    }

    if (_input_active) {
        SDL_Color yellow{ 255, 220, 120, 255 };
        std::string prompt = "> " + _input_buffer + "_";
        draw_text(prompt, x, cy + 4, yellow);
    } else {
        SDL_Color gray{ 180, 180, 180, 255 };
        draw_text("[Enter] para chatear  (ej: /meditar, /tomar, @nick hola)", x, cy + 4, gray);
        (void)screen_w;
    }
}
