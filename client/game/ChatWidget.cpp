#include "ChatWidget.h"
#include <stdexcept>
#include <sstream>

// Ancho del chat en pixels
static constexpr int CHAT_W     = 620;
static constexpr int CHAT_PAD   = 8;
static constexpr int TEXT_MAX_W = CHAT_W - CHAT_PAD * 2;

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
        return true;
    }
    if (e.type == SDL_TEXTINPUT) {
        _input_buffer += e.text.text;
        return true;
    }
    return false;
}

void ChatWidget::add_message(const std::string& text) {
    Uint32 now = SDL_GetTicks();
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line, '\n')) {
        _messages.push_back({ line, now });
    }
    while (_messages.size() > 60) _messages.pop_front();
}

void ChatWidget::render(int screen_w, int screen_h) {
    const int line_h = _font_size + 4;
    const int x      = CHAT_PAD;
    const int y      = CHAT_PAD;
    const int max_lines = (screen_h / 3) / line_h;  // máximo 1/3 de pantalla

    Uint32 now = SDL_GetTicks();

    // Eliminar mensajes vencidos (solo si el input no está activo)
    if (!_input_active) {
        while (!_messages.empty() &&
               now - _messages.front().born_ms > MSG_LIFETIME_MS) {
            _messages.pop_front();
        }
    }

    // Calcular líneas wrapeadas solo de mensajes vigentes
    std::vector<std::pair<std::vector<std::string>, float>> all_wrapped;
    int total_lines = 0;
    for (const auto& msg : _messages) {
        auto wrapped = wrap_text(msg.text);
        if (wrapped.empty()) wrapped.push_back("");

        // Calcular alpha según tiempo de vida (fade-out en el último segundo)
        float elapsed = static_cast<float>(now - msg.born_ms);
        float alpha   = 1.0f;
        float fade_start = static_cast<float>(MSG_LIFETIME_MS) - 1000.0f;
        if (elapsed > fade_start)
            alpha = 1.0f - (elapsed - fade_start) / 1000.0f;
        alpha = std::max(0.0f, std::min(1.0f, alpha));

        total_lines += static_cast<int>(wrapped.size());
        all_wrapped.push_back({ std::move(wrapped), alpha });
    }

    // Si no hay mensajes y el input no está activo, solo mostrar el hint pequeño
    bool has_content = !_messages.empty() || _input_active;

    if (has_content) {
        int lines_to_skip = std::max(0, total_lines - max_lines);
        int visible_lines = std::min(total_lines, max_lines);
        const int box_h   = line_h * visible_lines + (_input_active ? line_h + 6 : 0) + CHAT_PAD * 2;

        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer.Get(), 0, 0, 0, 120);
        SDL_Rect bg{ x - CHAT_PAD, y - CHAT_PAD, CHAT_W, box_h };
        SDL_RenderFillRect(_renderer.Get(), &bg);

        int cy    = y;
        int skipped = 0;
        for (const auto& [lines, alpha] : all_wrapped) {
            uint8_t a = static_cast<uint8_t>(255 * alpha);
            SDL_Color col{ 255, 255, 255, a };
            for (const auto& line : lines) {
                if (skipped < lines_to_skip) { skipped++; continue; }
                draw_text(line, x, cy, col);
                cy += line_h;
            }
        }

        if (_input_active) {
            SDL_Color yellow{ 255, 220, 120, 255 };
            draw_text("> " + _input_buffer + "_", x, cy + 4, yellow);
        }
    }

    // Hint siempre visible abajo del área (pequeño, sin fondo)
    if (!_input_active) {
        int hint_y = has_content
            ? y + std::min(total_lines, max_lines) * line_h + CHAT_PAD + 4
            : y;
        SDL_Color gray{ 140, 140, 140, 160 };
        draw_text("[Enter] para chatear", x, hint_y, gray);
        (void)screen_w;
    }
}

// Word-wrap
// Divide 'text' en líneas que entren en TEXT_MAX_W pixels.
// Respeta '\n' como salto de línea forzado.
std::vector<std::string> ChatWidget::wrap_text(const std::string& text) const {
    std::vector<std::string> lines;
    if (!_font || text.empty()) return lines;

    // Primero dividir por '\n' explícitos
    std::vector<std::string> paragraphs;
    std::istringstream ss(text);
    std::string para;
    while (std::getline(ss, para, '\n'))
        paragraphs.push_back(para);

    for (const auto& paragraph : paragraphs) {
        if (paragraph.empty()) { lines.push_back(""); continue; }

        std::string current;
        std::istringstream words(paragraph);
        std::string word;

        while (words >> word) {
            std::string candidate = current.empty() ? word : current + " " + word;
            int w = 0, h = 0;
            TTF_SizeUTF8(_font, candidate.c_str(), &w, &h);
            if (w <= TEXT_MAX_W) {
                current = candidate;
            } else {
                // La palabra no entra — guardar línea actual y empezar nueva
                if (!current.empty()) lines.push_back(current);

                // Si la palabra sola es más ancha que el ancho, cortarla por caracteres
                int ww = 0;
                TTF_SizeUTF8(_font, word.c_str(), &ww, &h);
                if (ww > TEXT_MAX_W) {
                    std::string chunk;
                    for (char c : word) {
                        std::string test = chunk + c;
                        TTF_SizeUTF8(_font, test.c_str(), &ww, &h);
                        if (ww > TEXT_MAX_W) {
                            lines.push_back(chunk);
                            chunk = std::string(1, c);
                        } else {
                            chunk = test;
                        }
                    }
                    current = chunk;
                } else {
                    current = word;
                }
            }
        }
        if (!current.empty()) lines.push_back(current);
    }
    return lines;
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
