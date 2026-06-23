#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <deque>
#include <string>
#include <functional>

struct ChatLine {
    std::string text;
    Uint32      born_ms;
};

class ChatWidget {
public:
    ChatWidget(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size = 14);
    ~ChatWidget();

    // Devuelve true si el evento fue consumido (el GameLoop debe ignorarlo).
    bool handle_event(const SDL_Event& e);

    void add_message(const std::string& text);
    void render(int screen_w, int screen_h);

    bool input_active() const { return _input_active; }

    // Callback invocado cuando el usuario aprieta Enter con texto.
    void on_submit(std::function<void(const std::string&)> cb) { _on_submit = std::move(cb); }

private:
    SDL2pp::Renderer&  _renderer;
    TTF_Font*          _font = nullptr;
    int                _font_size;

    std::deque<ChatLine> _messages;

    static constexpr Uint32 MSG_LIFETIME_MS = 12000; // 12 segundos por mensaje
    std::string             _input_buffer;
    bool                    _input_active = false;

    std::function<void(const std::string&)> _on_submit;

    void draw_text(const std::string& text, int x, int y, SDL_Color color);

    std::vector<std::string> wrap_text(const std::string& text) const;
};
