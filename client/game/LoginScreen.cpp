#include "LoginScreen.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include "../config/ClientConfig.h"

//  Razas/clases

const RacesClassesConfig::Race& LoginScreen::race_info(uint8_t idx) {
    return RacesClassesConfig::instance().get_race(idx + 1);
}

const RacesClassesConfig::Class& LoginScreen::class_info(uint8_t idx) {
    return RacesClassesConfig::instance().get_class(idx + 1);
}

//  Constructor / destructor

LoginScreen::LoginScreen(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
                         const std::string& font_path)
    : _window(window), _renderer(renderer) {
    if (TTF_WasInit() == 0) TTF_Init();
    const auto& fonts = ClientConfig::instance().fonts;
    _font_lg = TTF_OpenFont(font_path.c_str(), fonts.title_font_size);
    _font_md = TTF_OpenFont(font_path.c_str(), fonts.medium_font_size);
    _font_sm = TTF_OpenFont(font_path.c_str(), fonts.small_font_size);

    // SDL text input
    SDL_StartTextInput();
    std::memset(_race_tex, 0, sizeof(_race_tex));
}

LoginScreen::~LoginScreen() {
    SDL_StopTextInput();
    if (_font_lg) TTF_CloseFont(_font_lg);
    if (_font_md) TTF_CloseFont(_font_md);
    if (_font_sm) TTF_CloseFont(_font_sm);
    for (auto* t : _race_tex) if (t) SDL_DestroyTexture(t);
}

void LoginScreen::set_error(const std::string& msg) { _error_msg = msg; }

//  run

LoginResult LoginScreen::run() {
    _running   = true;
    _cancelled = false;

    while (_running) {
        handle_events();
        render();
        SDL_Delay(16);
    }

    LoginResult res;
    res.cancelled    = _cancelled;
    res.do_register  = (_screen == Screen::REGISTER_FORM ||
                        _screen == Screen::REGISTER_CLASS ||
                        _screen == Screen::REGISTER_RACE)
                       ? true : false;
    res.username     = _username_input;
    res.race         = _sel_race;
    res.cls          = _sel_class;
    return res;
}

//  handle_events

void LoginScreen::handle_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { _cancelled = true; _running = false; return; }

        if (e.type == SDL_MOUSEMOTION) {
            _hover_x = e.motion.x; _hover_y = e.motion.y;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            handle_mouse_click(e.button.x, e.button.y);
        }

        if (e.type == SDL_KEYDOWN) {
            handle_key(e.key);
        }

        // Texto libre
        if (e.type == SDL_TEXTINPUT && _username_active) {
            if (_username_input.size() < 20)
                _username_input += e.text.text;
        }
    }
}

static bool in_rect(int mx, int my, const SDL_Rect& r) {
    return mx >= r.x && mx < r.x+r.w && my >= r.y && my < r.y+r.h;
}

void LoginScreen::handle_mouse_click(int mx, int my) {
    _error_msg.clear();

    switch (_screen) {
    case Screen::MAIN:
        if (in_rect(mx, my, _btn_login)) {
            _screen = Screen::LOGIN_FORM;
            _username_input.clear();
            _username_active = true;
        }
        if (in_rect(mx, my, _btn_register)) {
            _screen = Screen::REGISTER_RACE;
            _username_input.clear();
            _race_chosen = _class_chosen = false;
        }
        break;

    case Screen::LOGIN_FORM:
        if (in_rect(mx, my, _btn_back))    { _screen = Screen::MAIN; }
        if (in_rect(mx, my, _btn_confirm)) {
            if (!_username_input.empty()) {
                _running = false;  // devolver al caller con login
            } else {
                _error_msg = "Ingresa un nombre de usuario.";
            }
        }
        break;

    case Screen::REGISTER_RACE:
        for (int i = 0; i < 4; i++) {
            if (in_rect(mx, my, _race_cards[i])) {
                _sel_race = static_cast<uint8_t>(i);
                _race_chosen = true;
            }
        }
        if (in_rect(mx, my, _btn_back)) { _screen = Screen::MAIN; }
        if (in_rect(mx, my, _btn_confirm) && _race_chosen) {
            _screen = Screen::REGISTER_CLASS;
        }
        if (!_race_chosen && in_rect(mx, my, _btn_confirm)) {
            _error_msg = "Selecciona una raza.";
        }
        break;

    case Screen::REGISTER_CLASS:
        for (int i = 0; i < 4; i++) {
            if (in_rect(mx, my, _class_cards[i])) {
                _sel_class = static_cast<uint8_t>(i);
                _class_chosen = true;
            }
        }
        if (in_rect(mx, my, _btn_back)) { _screen = Screen::REGISTER_RACE; }
        if (in_rect(mx, my, _btn_confirm) && _class_chosen) {
            _screen = Screen::REGISTER_FORM;
            _username_input.clear();
            _username_active = true;
        }
        if (!_class_chosen && in_rect(mx, my, _btn_confirm)) {
            _error_msg = "Selecciona una clase.";
        }
        break;

    case Screen::REGISTER_FORM:
        if (in_rect(mx, my, _btn_back)) { _screen = Screen::REGISTER_CLASS; }
        if (in_rect(mx, my, _btn_confirm)) {
            if (!_username_input.empty()) {
                _running = false;  // devolver al caller con registro
            } else {
                _error_msg = "Ingresa un nombre de usuario.";
            }
        }
        break;
    }
}

void LoginScreen::handle_key(const SDL_KeyboardEvent& k) {
    if (k.keysym.sym == SDLK_BACKSPACE && _username_active && !_username_input.empty()) {
        _username_input.pop_back();
    }
    if (k.keysym.sym == SDLK_RETURN) {
        // Simula click en confirm
        handle_mouse_click(_btn_confirm.x + 1, _btn_confirm.y + 1);
    }
    if (k.keysym.sym == SDLK_ESCAPE) {
        if (_screen == Screen::MAIN) { _cancelled = true; _running = false; }
        else _screen = Screen::MAIN;
    }
}

//  Helpers de carga

SDL_Texture* LoginScreen::load_texture(const std::string& path) {
    SDL_Surface* s = IMG_Load(path.c_str());
    if (!s) return nullptr;
    SDL_Texture* t = SDL_CreateTextureFromSurface(_renderer.Get(), s);
    SDL_FreeSurface(s);
    return t;
}

//  Helpers de dibujo

void LoginScreen::draw_bg() {
    int w = _window.GetWidth(), h = _window.GetHeight();
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(_renderer.Get(), 8, 5, 3, 255);
    SDL_Rect full{0, 0, w, h};
    SDL_RenderFillRect(_renderer.Get(), &full);

    // Barra superior
    SDL_SetRenderDrawColor(_renderer.Get(), 20, 15, 8, 255);
    SDL_Rect top{0, 0, w, 50};
    SDL_RenderFillRect(_renderer.Get(), &top);
    SDL_SetRenderDrawColor(_renderer.Get(), 90, 65, 25, 255);
    SDL_RenderDrawLine(_renderer.Get(), 0, 50, w, 50);

    // Barra inferior
    SDL_Rect bot{0, h - 50, w, 50};
    SDL_SetRenderDrawColor(_renderer.Get(), 20, 15, 8, 255);
    SDL_RenderFillRect(_renderer.Get(), &bot);
    SDL_SetRenderDrawColor(_renderer.Get(), 90, 65, 25, 255);
    SDL_RenderDrawLine(_renderer.Get(), 0, h - 50, w, h - 50);
}

void LoginScreen::draw_logo() {
    int w = _window.GetWidth();
    SDL_Color gold{220, 185, 50, 255};
    SDL_Color red_c{180, 30, 30, 255};
    draw_text_centered("ARGENTUM ONLINE", w/2, 22, gold, _font_lg);
    draw_text_centered("Grupo 13", w/2, 36, red_c, _font_sm);
}

void LoginScreen::draw_text(const std::string& t, int x, int y,
                            SDL_Color c, TTF_Font* f) {
    if (t.empty()) return;
    TTF_Font* font = f ? f : _font_md;
    if (!font) return;
    // Soporte para \n
    std::string line;
    int ly = y;
    int lh = TTF_FontLineSkip(font);
    auto flush = [&]() {
        if (line.empty()) return;
        SDL_Surface* s = TTF_RenderUTF8_Blended(font, line.c_str(), c);
        if (!s) return;
        SDL_Texture* tx = SDL_CreateTextureFromSurface(_renderer.Get(), s);
        SDL_Rect dst{x, ly, s->w, s->h};
        SDL_RenderCopy(_renderer.Get(), tx, nullptr, &dst);
        SDL_DestroyTexture(tx);
        SDL_FreeSurface(s);
        line.clear();
        ly += lh;
    };
    for (char ch : t) {
        if (ch == '\n') flush();
        else line += ch;
    }
    flush();
}

void LoginScreen::draw_text_centered(const std::string& t, int cx, int cy,
                                     SDL_Color c, TTF_Font* f) {
    TTF_Font* font = f ? f : _font_md;
    if (!font || t.empty()) return;
    int w = 0, h = 0;
    TTF_SizeUTF8(font, t.c_str(), &w, &h);
    draw_text(t, cx - w/2, cy - h/2, c, font);
}

void LoginScreen::draw_panel(int x, int y, int w, int h) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 18, 13, 7, 240);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(_renderer.Get(), &r);
    SDL_SetRenderDrawColor(_renderer.Get(), 90, 65, 25, 255);
    SDL_RenderDrawRect(_renderer.Get(), &r);
    SDL_SetRenderDrawColor(_renderer.Get(), 45, 33, 13, 255);
    SDL_Rect inner{x+1, y+1, w-2, h-2};
    SDL_RenderDrawRect(_renderer.Get(), &inner);
}

void LoginScreen::draw_button(const SDL_Rect& r, const std::string& label,
                              bool hovered, bool selected) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_Color bg  = selected  ? SDL_Color{100, 20, 20, 240}
                  : hovered   ? SDL_Color{ 65, 50, 18, 240}
                  :             SDL_Color{ 35, 28, 10, 230};
    SDL_SetRenderDrawColor(_renderer.Get(), bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(_renderer.Get(), &r);

    SDL_Color border = selected ? SDL_Color{200, 60, 60, 255}
                     : hovered  ? SDL_Color{180,140, 40, 255}
                     :            SDL_Color{ 90, 65, 25, 200};
    SDL_SetRenderDrawColor(_renderer.Get(), border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(_renderer.Get(), &r);

    // Highlight top
    SDL_SetRenderDrawColor(_renderer.Get(), 120, 90, 30, 80);
    SDL_RenderDrawLine(_renderer.Get(), r.x+1, r.y+1, r.x+r.w-2, r.y+1);

    SDL_Color tc = selected ? SDL_Color{255,200,200,255}
                 : hovered  ? SDL_Color{255,220,100,255}
                 :            SDL_Color{200,180,120,255};
    draw_text_centered(label, r.x + r.w/2, r.y + r.h/2, tc, _font_md);
}

void LoginScreen::draw_input(const SDL_Rect& r, const std::string& text,
                              bool active, const std::string& placeholder) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_Color bg = active ? SDL_Color{28, 22, 10, 255} : SDL_Color{18, 14, 7, 255};
    SDL_SetRenderDrawColor(_renderer.Get(), bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(_renderer.Get(), &r);
    SDL_Color border = active ? SDL_Color{180,140,40,255} : SDL_Color{70,55,25,200};
    SDL_SetRenderDrawColor(_renderer.Get(), border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(_renderer.Get(), &r);

    if (text.empty() && !placeholder.empty()) {
        draw_text(placeholder, r.x+8, r.y+(r.h-14)/2, SDL_Color{80,70,50,255}, _font_md);
    } else {
        std::string display = text;
        if (active && (SDL_GetTicks() / 500) % 2 == 0) display += "|";
        draw_text(display, r.x+8, r.y+(r.h-14)/2, SDL_Color{230,210,160,255}, _font_md);
    }
}

// Race card
void LoginScreen::draw_race_card(int idx, int x, int y, int w, int h) {
    _race_cards[idx] = {x, y, w, h};
    bool selected = (_race_chosen && _sel_race == idx);
    bool hovered  = in_rect(_hover_x, _hover_y, _race_cards[idx]);

    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_Color bg = selected ? SDL_Color{60,40,10,240}
                 : hovered  ? SDL_Color{40,30, 8,230}
                 :            SDL_Color{22,16, 6,220};
    SDL_SetRenderDrawColor(_renderer.Get(), bg.r, bg.g, bg.b, bg.a);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(_renderer.Get(), &r);

    SDL_Color border = selected ? SDL_Color{220,170,40,255}
                     : hovered  ? SDL_Color{130,100,30,200}
                     :            SDL_Color{ 70, 55,25,180};
    SDL_SetRenderDrawColor(_renderer.Get(), border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(_renderer.Get(), &r);

    // Imagen del sprite: cargar spritesheet con IMG_Load (PNG)
    if (!_race_tex[idx]) {
        SDL_Surface* surf = IMG_Load(race_info(static_cast<uint8_t>(idx)).sprite_path.c_str());
        if (surf) {
            _race_tex[idx] = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
            SDL_FreeSurface(surf);
        }
    }

    if (_race_tex[idx]) {
        int tw = 0, th = 0;
        SDL_QueryTexture(_race_tex[idx], nullptr, nullptr, &tw, &th);
        int cols = 4;
        int fw = tw / cols;
        int fh = th / 5;  // aprox
        SDL_Rect src{0, 0, fw, fh};
        int img_w = w - 20, img_h = h - 55;
        // Mantener aspecto
        float aspect = static_cast<float>(fw) / fh;
        if (img_w > img_h * aspect) img_w = static_cast<int>(img_h * aspect);
        else img_h = static_cast<int>(img_w / aspect);
        SDL_Rect dst{x + (w - img_w)/2, y + 10, img_w, img_h};
        SDL_RenderCopy(_renderer.Get(), _race_tex[idx], &src, &dst);
    } else {
        // Placeholder de color con inicial
        SDL_Color race_colors[4] = {
            {180,140,100,255},{120,180,130,255},{160,120,80,255},{140,120,160,255}
        };
        SDL_Color rc = race_colors[idx];
        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer.Get(), rc.r/3, rc.g/3, rc.b/3, 200);
        SDL_Rect img_r{x+8, y+8, w-16, h-52};
        SDL_RenderFillRect(_renderer.Get(), &img_r);
        // Dibujar inicial centrada
        std::string initial(1, race_info(static_cast<uint8_t>(idx)).name[0]);
        draw_text_centered(initial, x+w/2, y+(h-52)/2+8,
                           SDL_Color{rc.r,rc.g,rc.b,255}, _font_lg);
    }

    // Nombre de raza
    SDL_Color name_c = selected ? SDL_Color{255,210,80,255}
                     : hovered  ? SDL_Color{220,190,100,255}
                     :            SDL_Color{170,150,90,255};
    draw_text_centered(race_info(static_cast<uint8_t>(idx)).name, x+w/2, y+h-32, name_c, _font_md);

    // Indicador seleccionado
    if (selected) {
        SDL_SetRenderDrawColor(_renderer.Get(), 200,160,40,255);
        SDL_Rect sel_r{x+2, y+2, w-4, h-4};
        SDL_RenderDrawRect(_renderer.Get(), &sel_r);
    }
}

// Class card
void LoginScreen::draw_class_card(int idx, int x, int y, int w, int h) {
    _class_cards[idx] = {x, y, w, h};
    bool selected = (_class_chosen && _sel_class == idx);
    bool hovered  = in_rect(_hover_x, _hover_y, _class_cards[idx]);

    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_Color bg = selected ? SDL_Color{50,20,60,240}
                 : hovered  ? SDL_Color{35,15,45,230}
                 :            SDL_Color{22,10,28,220};
    SDL_SetRenderDrawColor(_renderer.Get(), bg.r, bg.g, bg.b, bg.a);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(_renderer.Get(), &r);

    SDL_Color border = selected ? SDL_Color{180,100,220,255}
                     : hovered  ? SDL_Color{110, 60,150,200}
                     :            SDL_Color{ 70, 40,100,180};
    SDL_SetRenderDrawColor(_renderer.Get(), border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(_renderer.Get(), &r);

    // Icono por clase (colores distintos)
    SDL_Color class_icons[4] = {
        {80,120,220,255},  // Mago - azul
        {80,200,130,255},  // Clerigo - verde
        {220,180,40,255},  // Paladin - dorado
        {200,60,60,255},   // Guerrero - rojo
    };
    SDL_Color ic = class_icons[idx];
    SDL_SetRenderDrawColor(_renderer.Get(), ic.r/3, ic.g/3, ic.b/3, 200);
    SDL_Rect icon_r{x+w/2-20, y+12, 40, 40};
    SDL_RenderFillRect(_renderer.Get(), &icon_r);
    SDL_SetRenderDrawColor(_renderer.Get(), ic.r, ic.g, ic.b, 255);
    SDL_RenderDrawRect(_renderer.Get(), &icon_r);
    // Inicial de clase
    const auto& cls = class_info(static_cast<uint8_t>(idx));
    std::string cl_init(1, cls.name[0]);
    draw_text_centered(cl_init, x+w/2, y+32, SDL_Color{ic.r,ic.g,ic.b,255}, _font_lg);

    // Nombre
    SDL_Color nc = selected ? SDL_Color{220,170,255,255}
                 : hovered  ? SDL_Color{190,150,220,255}
                 :            SDL_Color{150,120,180,255};
    draw_text_centered(cls.name, x+w/2, y+60, nc, _font_md);

    // Clip al rect de la card para que el texto no se derrame a otras cards
    SDL_Rect clip{x+4, y+68, w-8, h-72};
    SDL_RenderSetClipRect(_renderer.Get(), &clip);

    // Descripción (multi-línea dentro del clip)
    draw_text(cls.flavor, x+6, y+76, SDL_Color{170,160,150,255}, _font_sm);
    draw_text(cls.benefits, x+6, y+h-44, SDL_Color{120,160,120,255}, _font_sm);

    SDL_RenderSetClipRect(_renderer.Get(), nullptr);  // desactivar clip

    if (selected) {
        SDL_SetRenderDrawColor(_renderer.Get(), 160,80,200,255);
        SDL_Rect sel_r{x+2, y+2, w-4, h-4};
        SDL_RenderDrawRect(_renderer.Get(), &sel_r);
    }
}

//  render

void LoginScreen::render() {
    _renderer.Clear();
    draw_bg();
    draw_logo();

    switch (_screen) {
        case Screen::MAIN:            render_main();          break;
        case Screen::LOGIN_FORM:      render_login_form();    break;
        case Screen::REGISTER_RACE:   render_register_race(); break;
        case Screen::REGISTER_CLASS:  render_register_class();break;
        case Screen::REGISTER_FORM:   render_register_form(); break;
    }

    // Error (común a todas las pantallas)
    if (!_error_msg.empty()) {
        int w = _window.GetWidth(), h = _window.GetHeight();
        draw_text_centered(_error_msg, w/2, h - 30,
                           SDL_Color{220,80,80,255}, _font_md);
    }

    _renderer.Present();
}

// Pantalla principal
void LoginScreen::render_main() {
    int sw = _window.GetWidth(), sh = _window.GetHeight();

    // Panel central
    const int PW = 300, PH = 180;
    int px = (sw - PW)/2, py = (sh - PH)/2;
    draw_panel(px, py, PW, PH);

    SDL_Color title{200,175,100,255};
    draw_text_centered("Bienvenido", px+PW/2, py+20, title, _font_md);

    const int BW = 220, BH = 40;
    _btn_login    = {px + (PW-BW)/2, py+50,  BW, BH};
    _btn_register = {px + (PW-BW)/2, py+105, BW, BH};

    draw_button(_btn_login,    "Iniciar sesion",
                in_rect(_hover_x,_hover_y,_btn_login));
    draw_button(_btn_register, "Registrarse",
                in_rect(_hover_x,_hover_y,_btn_register));

    draw_text_centered("ESC para salir", sw/2, sh-25,
                       SDL_Color{80,70,50,255}, _font_sm);
}

// Login form
void LoginScreen::render_login_form() {
    int sw = _window.GetWidth(), sh = _window.GetHeight();
    const int PW = 360, PH = 160;
    int px = (sw-PW)/2, py = (sh-PH)/2;
    draw_panel(px, py, PW, PH);

    SDL_Color title{200,175,100,255};
    draw_text_centered("Iniciar sesion", px+PW/2, py+18, title, _font_md);

    draw_text("Nombre de usuario:", px+16, py+44, SDL_Color{160,145,100,255}, _font_sm);
    SDL_Rect input_r{px+16, py+60, PW-32, 30};
    draw_input(input_r, _username_input, _username_active, "Escribe tu usuario...");

    const int BW = 110, BH = 32;
    _btn_back    = {px+16,         py+PH-48, BW, BH};
    _btn_confirm = {px+PW-16-BW,   py+PH-48, BW, BH};

    draw_button(_btn_back,    "< Volver",  in_rect(_hover_x,_hover_y,_btn_back));
    draw_button(_btn_confirm, "Entrar",    in_rect(_hover_x,_hover_y,_btn_confirm));
}

// Register race
void LoginScreen::render_register_race() {
    int sw = _window.GetWidth(), sh = _window.GetHeight();

    SDL_Color title{200,175,100,255};
    draw_text_centered("Elige tu raza", sw/2, 70, title, _font_lg);

    // 4 cards en una fila
    const int CARD_W = 155, CARD_H = 200;
    int total_w = 4*CARD_W + 3*12;
    int gx0 = (sw - total_w)/2;
    int gy0 = 95;

    for (int i = 0; i < 4; i++) {
        draw_race_card(i, gx0 + i*(CARD_W+12), gy0, CARD_W, CARD_H);
    }

    // Descripción de la raza seleccionada
    if (_race_chosen) {
        int dy = gy0 + CARD_H + 12;
        draw_panel((sw-580)/2, dy, 580, 52);
        draw_text(race_info(_sel_race).description, (sw-580)/2+10, dy+8,
                  SDL_Color{180,165,120,255}, _font_sm);
    }

    const int BW = 120, BH = 34;
    int by = sh - 70;
    _btn_back    = {(sw/2) - BW - 10, by, BW, BH};
    _btn_confirm = {(sw/2) + 10,      by, BW, BH};

    draw_button(_btn_back,    "< Volver",   in_rect(_hover_x,_hover_y,_btn_back));
    draw_button(_btn_confirm, "Siguiente >",in_rect(_hover_x,_hover_y,_btn_confirm),
                !_race_chosen);
}

// Register class
void LoginScreen::render_register_class() {
    int sw = _window.GetWidth(), sh = _window.GetHeight();

    SDL_Color title{200,175,100,255};
    draw_text_centered("Elige tu clase", sw/2, 70, title, _font_lg);

    // Calcular ancho de card dinámicamente según la ventana, con margen lateral
    const int MARGIN = 16;
    const int GAP = 10;
    const int CARD_W = (sw - MARGIN*2 - GAP*3) / 4;
    const int CARD_H = std::min(sh - 95 - 90, 340);  // dejar espacio para botones
    int gx0 = MARGIN;
    int gy0 = 95;

    for (int i = 0; i < 4; i++) {
        draw_class_card(i, gx0 + i*(CARD_W+GAP), gy0, CARD_W, CARD_H);
    }

    const int BW = 120, BH = 34;
    int by = sh - 70;
    _btn_back    = {(sw/2) - BW - 10, by, BW, BH};
    _btn_confirm = {(sw/2) + 10,      by, BW, BH};

    draw_button(_btn_back,    "< Volver",   in_rect(_hover_x,_hover_y,_btn_back));
    draw_button(_btn_confirm, "Siguiente >",in_rect(_hover_x,_hover_y,_btn_confirm),
                !_class_chosen);
}

// Register form
void LoginScreen::render_register_form() {
    int sw = _window.GetWidth(), sh = _window.GetHeight();
    const int PW = 400, PH = 195;
    int px = (sw-PW)/2, py = (sh-PH)/2;
    draw_panel(px, py, PW, PH);

    SDL_Color title{200,175,100,255};
    draw_text_centered("Crear personaje", px+PW/2, py+18, title, _font_md);

    // Resumen de elección
    std::string summary = std::string("Raza: ") + race_info(_sel_race).name +
                          "   Clase: " + class_info(_sel_class).name;
    draw_text_centered(summary, px+PW/2, py+38, SDL_Color{150,180,150,255}, _font_sm);

    draw_text("Nombre de usuario:", px+16, py+58, SDL_Color{160,145,100,255}, _font_sm);
    SDL_Rect input_r{px+16, py+74, PW-32, 30};
    draw_input(input_r, _username_input, _username_active, "Elige un nombre...");

    draw_text("(max 20 caracteres, sin espacios)",
              px+16, py+108, SDL_Color{80,75,60,255}, _font_sm);

    const int BW = 130, BH = 32;
    _btn_back    = {px+16,       py+PH-48, BW, BH};
    _btn_confirm = {px+PW-16-BW, py+PH-48, BW, BH};

    draw_button(_btn_back,    "< Volver",      in_rect(_hover_x,_hover_y,_btn_back));
    draw_button(_btn_confirm, "Crear cuenta",  in_rect(_hover_x,_hover_y,_btn_confirm));
}
