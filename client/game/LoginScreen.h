#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_image.h>
#include <string>
#include <array>
#include <atomic>

#include "../../common/queue.h"
#include "../net/Command.h"
#include "../config/RacesClassesConfig.h"

class AudioManager;

struct LoginResult {
    bool     cancelled = false;  // el usuario cerró la ventana
    bool     do_register = false;
    std::string username;
    uint8_t  race = 0;
    uint8_t  cls  = 0;
};

class LoginScreen {
public:
    LoginScreen(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
                const std::string& font_path, AudioManager* audio = nullptr);
    ~LoginScreen();

    // Muestra un mensaje de error (llamar antes de run() para reintento)
    void set_error(const std::string& msg);

    // Bucle principal. Devuelve cuando el usuario confirma o cierra la ventana.
    LoginResult run();

private:
    // Estados de la pantalla
    enum class Screen { SPLASH, MAIN, LOGIN_FORM, REGISTER_RACE, REGISTER_CLASS, REGISTER_FORM };

    // Render
    void render();
    void render_splash();
    void render_main();
    void render_login_form();
    void render_register_race();
    void render_register_class();
    void render_register_form();

    // Eventos
    void handle_events();
    void handle_key(const SDL_KeyboardEvent& k);
    void handle_mouse_click(int mx, int my);

    // Chequea si (mx,my) cae dentro de r; si es así, reproduce el sonido de
    // click. 
    bool clicked(int mx, int my, const SDL_Rect& r);

    // Helpers de dibujo
    void draw_bg();
    void draw_logo();
    void draw_text(const std::string& t, int x, int y, SDL_Color c,
                   TTF_Font* f = nullptr);
    void draw_text_centered(const std::string& t, int cx, int cy,
                            SDL_Color c, TTF_Font* f = nullptr);
    void draw_button(const SDL_Rect& r, const std::string& label,
                     bool hovered, bool selected = false);
    void draw_input(const SDL_Rect& r, const std::string& text,
                    bool active, const std::string& placeholder = "");
    void draw_race_card(int idx, int x, int y, int w, int h);
    void draw_class_card(int idx, int x, int y, int w, int h);
    void draw_panel(int x, int y, int w, int h);
    SDL_Texture* load_texture(const std::string& path);

    SDL2pp::Window&   _window;
    SDL2pp::Renderer& _renderer;
    AudioManager*     _audio = nullptr;
    TTF_Font* _font_lg = nullptr;   // 20px
    TTF_Font* _font_md = nullptr;   // 14px
    TTF_Font* _font_sm = nullptr;   // 11px

    Screen      _screen  = Screen::SPLASH;
    bool        _running = true;
    bool        _cancelled = false;

    // Formulario
    std::string _username_input;
    bool        _username_active = false;
    uint8_t     _sel_race = 0;
    uint8_t     _sel_class = 0;
    bool        _race_chosen  = false;
    bool        _class_chosen = false;

    // Error del servidor
    std::string _error_msg;

    // Hover tracking
    int _hover_x = 0, _hover_y = 0;

    // Texturas de sprites de raza 
    SDL_Texture* _race_tex[4] {};
    // Logo grande de la pantalla de splash 
    SDL_Texture* _logo_tex = nullptr;

    // Rects de botones calculados en render, usados en handle_mouse_click
    SDL_Rect _btn_comenzar {};
    SDL_Rect _btn_login    {};
    SDL_Rect _btn_register {};
    SDL_Rect _btn_back     {};
    SDL_Rect _btn_confirm  {};
    SDL_Rect _race_cards[4]{};
    SDL_Rect _class_cards[4]{};

    // Datos de razas/clases
    static const RacesClassesConfig::Race&  race_info(uint8_t idx);
    static const RacesClassesConfig::Class& class_info(uint8_t idx);
};
