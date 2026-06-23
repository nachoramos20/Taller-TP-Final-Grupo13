#include "StatsPanelRenderer.h"

#include <algorithm>
#include <cmath>

#include "../../common/protocol/WeaponRules.h"

StatsPanelRenderer::StatsPanelRenderer(SDL2pp::Renderer& renderer, TTF_Font* font, int font_size):
        _renderer(renderer), _font(font), _font_size(font_size) {}

void StatsPanelRenderer::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty() || !_font)
        return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(_font, text.c_str(), color);
    if (!surf)
        return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
    SDL_Rect dst{x, y, surf->w, surf->h};
    SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void StatsPanelRenderer::draw_text_right(const std::string& text, int right_x, int y,
                                         SDL_Color color) {
    if (text.empty() || !_font)
        return;
    int text_w = 0, text_h = 0;
    TTF_SizeUTF8(_font, text.c_str(), &text_w, &text_h);
    draw_text(text, right_x - text_w, y, color);
}

void StatsPanelRenderer::draw_bar(int x, int y, int w, int h, float fraction, SDL_Color fill,
                                  SDL_Color bg) {
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), bg.r, bg.g, bg.b, bg.a);
    SDL_Rect bg_rect{x, y, w, h};
    SDL_RenderFillRect(_renderer.Get(), &bg_rect);
    int filled_w = static_cast<int>(w * fraction);
    if (filled_w > 0) {
        SDL_SetRenderDrawColor(_renderer.Get(), fill.r, fill.g, fill.b, fill.a);
        SDL_Rect fill_rect{x, y, filled_w, h};
        SDL_RenderFillRect(_renderer.Get(), &fill_rect);
    }
    SDL_SetRenderDrawColor(_renderer.Get(), 80, 80, 80, 200);
    SDL_RenderDrawRect(_renderer.Get(), &bg_rect);
}

void StatsPanelRenderer::draw_rounded_rect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), color.r, color.g, color.b, color.a);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(_renderer.Get(), &r);
    SDL_SetRenderDrawColor(_renderer.Get(), std::min(255, color.r + 50),
                           std::min(255, color.g + 50), std::min(255, color.b + 50), 220);
    SDL_RenderDrawRect(_renderer.Get(), &r);
}

StatsLayout StatsPanelRenderer::render(int screen_w, int screen_h, const StatsRenderInput& input) {
    StatsLayout layout;

    const int PW = PANEL_W;
    const int px = screen_w - PW;
    const int pad = 12;
    const int lh = _font_size + 6;

    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 12, 8, 4, 220);
    SDL_Rect bg{px, 0, PW, screen_h};
    SDL_RenderFillRect(_renderer.Get(), &bg);

    // Borde superior del panel
    SDL_SetRenderDrawColor(_renderer.Get(), 180, 140, 60, 255);
    SDL_RenderDrawLine(_renderer.Get(), px, 0, px + PW, 0);
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 70, 30, 255);
    SDL_RenderDrawLine(_renderer.Get(), px, 1, px + PW, 1);

    int cy = pad + 4;
    SDL_Color white{245, 245, 220, 255};
    SDL_Color gold_c{255, 200, 50, 255};
    SDL_Color ghost_c{180, 200, 255, 255};
    SDL_Color dim{120, 110, 90, 255};

    // ─── Header: Username + Nivel ───
    if (input.username && !input.username->empty()) {
        SDL_Color name_col = input.is_ghost ? ghost_c : SDL_Color{255, 220, 120, 255};
        draw_text(*input.username, px + pad, cy, name_col);
        cy += lh - 2;
    }

    std::string lvl_str = "Nivel " + std::to_string(input.level);
    draw_text(lvl_str, px + pad, cy, input.is_ghost ? ghost_c : gold_c);

    // Badge de estado a la derecha
    if (input.is_ghost || input.meditating) {
        std::string badge = input.is_ghost ? "[FANTASMA]" : "[MEDITANDO]";
        SDL_Color badge_col = input.is_ghost ? ghost_c : SDL_Color{100, 200, 100, 255};
        draw_text_right(badge, px + PW - pad, cy, badge_col);
    }
    cy += lh + 8;

    // Divisor visual
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
    SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
    cy += 6;

    // ─── HP ───
    draw_text("Vida", px + pad, cy, white);
    std::string hp_str = std::to_string(input.hp) + " / " + std::to_string(input.max_hp);
    draw_text_right(hp_str, px + PW - pad, cy, {220, 220, 220, 255});
    cy += lh - 2;

    float hp_frac = static_cast<float>(input.hp) / input.max_hp;
    SDL_Color hp_fill = hp_frac > 0.5f  ? SDL_Color{80, 220, 80, 255} :
                        hp_frac > 0.25f ? SDL_Color{255, 180, 0, 255} :
                                          SDL_Color{220, 50, 50, 255};
    draw_bar(px + pad, cy, PW - pad * 2, 20, hp_frac, hp_fill, {30, 10, 10, 180});
    cy += 20 + 10;

    // ─── Maná (si aplica) ───
    if (input.max_mp > 0) {
        draw_text("Maná", px + pad, cy, white);
        std::string mp_str = std::to_string(input.mp) + " / " + std::to_string(input.max_mp);
        draw_text_right(mp_str, px + PW - pad, cy, {200, 200, 220, 255});
        cy += lh - 2;

        float mp_frac = static_cast<float>(input.mp) / static_cast<float>(input.max_mp);
        draw_bar(px + pad, cy, PW - pad * 2, 20, mp_frac, {70, 130, 220, 255}, {15, 15, 50, 180});
        cy += 20 + 10;
    }

    // ─── EXP ───
    draw_text("Experiencia", px + pad, cy, white);

    uint32_t exp_current_level_floor =
            (input.level <= 1) ?
                    0 :
                    static_cast<uint32_t>(1000.0 *
                                          std::pow(static_cast<double>(input.level - 1), 1.8));

    uint32_t exp_next_level_ceil =
            (input.level >= 50) ?
                    0xFFFFFFFF :
                    static_cast<uint32_t>(1000.0 * std::pow(static_cast<double>(input.level), 1.8));

    uint32_t exp_needed_for_this_level = exp_next_level_ceil - exp_current_level_floor;

    uint32_t exp_progress_in_level = 0;
    if (input.exp > exp_current_level_floor)
        exp_progress_in_level = input.exp - exp_current_level_floor;

    if (exp_progress_in_level > exp_needed_for_this_level)
        exp_progress_in_level = exp_needed_for_this_level;

    std::string exp_str;
    if (input.level >= 50) {
        exp_str = "MAX";
    } else {
        exp_str = std::to_string(exp_progress_in_level) + " / " +
                  std::to_string(exp_needed_for_this_level);
    }

    draw_text_right(exp_str, px + PW - pad, cy, {200, 150, 220, 255});
    cy += lh - 2;

    float exp_frac = (input.level >= 50) ?
                             1.0f :
                             static_cast<float>(exp_progress_in_level) / exp_needed_for_this_level;
    draw_bar(px + pad, cy, PW - pad * 2, 16, exp_frac, {170, 120, 200, 255}, {40, 25, 60, 180});
    cy += 16 + 10;

    // ─── Oro ───
    draw_text("Oro", px + pad, cy, white);
    draw_text_right(std::to_string(input.gold), px + PW - pad, cy, gold_c);
    cy += lh + 10;

    // Divisor visual
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
    SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
    cy += 8;

    // ─── Botón Inventario ───
    const int btn_h = 36;
    const int btn_w = PW - pad * 2;
    layout.inv_btn_rect = {px + pad, cy, btn_w, btn_h};
    draw_rounded_rect(px + pad, cy, btn_w, btn_h, {100, 70, 30, 240});
    draw_text("Inventario  [Tab]", px + pad + (btn_w - 100) / 2, cy + (btn_h - _font_size) / 2,
              {255, 240, 160, 255});
    cy += btn_h + 10;

    layout.help_btn_rect = {px + pad, cy, btn_w, btn_h};
    draw_rounded_rect(px + pad, cy, btn_w, btn_h, {70, 70, 120, 240});
    draw_text("Ayuda [H]", px + pad + (btn_w - 90) / 2, cy + (btn_h - _font_size) / 2,
              {255, 255, 255, 255});
    cy += btn_h + 10;

    // ─── Sección Hechizos ───
    const auto& spells = *input.spells;

    // Divisor visual
    SDL_SetRenderDrawColor(_renderer.Get(), 100, 80, 40, 120);
    SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
    cy += 8;

    if (spells.empty()) {
        // Guerrero: sin hechizos. No hacemos return acá -- el popup de
        // ayuda (help_visible) se dibuja más abajo y antes cortaba la
        // función entera, dejando "Ayuda [H]" sin efecto visual para esta
        // clase aunque el toggle internamente sí cambiaba de estado.
        draw_text("Sin hechizos (Guerrero)", px + pad, cy, dim);
        cy += lh + 4;
    } else {
        draw_text("Hechizos", px + pad, cy, {200, 170, 240, 255});
        cy += lh + 4;

        bool magic_weapon = weapon_enables_spells(input.eq_weapon_item);
        if (!magic_weapon) {
            draw_text("Equipá báculo/vara", px + pad, cy, dim);
            cy += lh + 6;
        }

        // hechizos en dos líneas por botón
        const int sb_h = 8 + _font_size + 4 + _font_size + 8;
        const int sb_w = PW - pad * 2;
        for (size_t i = 0; i < spells.size() && (int)i < StatsLayout::MAX_SPELL_BTNS; i++) {
            const auto& s = spells[i];
            bool selected = input.cast_mode && input.selected_spell == s.id;
            bool enabled = magic_weapon && input.mp >= s.mana && !input.is_ghost;

            SDL_Color col = !enabled ? SDL_Color{50, 40, 60, 220} :
                            selected ? SDL_Color{130, 80, 180, 250} :
                                       SDL_Color{70, 50, 100, 230};
            draw_rounded_rect(px + pad, cy, sb_w, sb_h, col);

            SDL_Color txt = !enabled ? dim :
                            selected ? SDL_Color{255, 245, 200, 255} :
                                       SDL_Color{220, 200, 255, 255};

            // Nombre del hechizo
            std::string label = selected ? "* " : "  ";
            label += s.label;
            draw_text(label, px + pad + 8, cy + 8, txt);

            // Costo + atajo
            std::string shortcut_str = std::string("[") + std::to_string(i + 1) + "]";
            std::string cost_str = std::to_string(s.mana) + "mp " + shortcut_str;
            draw_text(cost_str, px + pad + 8, cy + 8 + _font_size + 4, dim);

            layout.spell_btn_rect[i] = {px + pad, cy, sb_w, sb_h};
            layout.spell_btn_id[i] = s.id;
            layout.spell_btn_count++;
            cy += sb_h + 6;
        }
    }

    if (input.cast_mode) {
        SDL_SetRenderDrawColor(_renderer.Get(), 100, 200, 100, 150);
        SDL_RenderDrawLine(_renderer.Get(), px + pad, cy, px + PW - pad, cy);
        cy += 6;
        draw_text("Modo hechizo ACTIVO", px + pad, cy, {150, 255, 150, 255});
        cy += lh + 4;
    } else {
        cy += 4;
    }

    if (input.help_visible) {
        render_help_overlay(screen_w, screen_h);
    }

    return layout;
}

void StatsPanelRenderer::render_help_overlay(int screen_w, int screen_h) {
    SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer.Get(), 0, 0, 0, 180);
    SDL_Rect overlay{0, 0, screen_w, screen_h};
    SDL_RenderFillRect(_renderer.Get(), &overlay);

    const int ww = 900;
    const int wh = 650;
    const int wx = (screen_w - ww) / 2;
    const int wy = (screen_h - wh) / 2;

    draw_rounded_rect(wx, wy, ww, wh, {25, 25, 35, 245});

    SDL_SetRenderDrawColor(_renderer.Get(), 255, 215, 100, 255);
    SDL_Rect border{wx, wy, ww, wh};
    SDL_RenderDrawRect(_renderer.Get(), &border);

    draw_text("AYUDA", wx + 20, wy + 20, {255, 220, 120, 255});

    SDL_Color gold{255, 200, 100, 255};
    SDL_Color white{245, 245, 220, 255};

    int left_x = wx + 25;
    int right_x = wx + ww / 2 + 15;
    int left_y = wy + 65;
    int right_y = wy + 65;

    // IZQUIERDA

    draw_text("COMANDOS", left_x, left_y, gold);
    left_y += 30;

    draw_text("/meditar", left_x, left_y, white);
    left_y += 22;
    draw_text("/resucitar", left_x, left_y, white);
    left_y += 22;
    draw_text("/curar", left_x, left_y, white);
    left_y += 22;
    draw_text("/info-mazmorra", left_x, left_y, white);
    left_y += 22;
    draw_text("/entrar-mazmorra", left_x, left_y, white);
    left_y += 22;
    draw_text("/salir-mazmorra", left_x, left_y, white);
    left_y += 40;

    draw_text("BANCO", left_x, left_y, gold);
    left_y += 30;

    draw_text("/listar", left_x, left_y, white);
    left_y += 22;
    draw_text("/depositar oro <cantidad>", left_x, left_y, white);
    left_y += 22;
    draw_text("/retirar oro <cantidad>", left_x, left_y, white);
    left_y += 22;
    draw_text("/depositar <objeto>", left_x, left_y, white);
    left_y += 22;
    draw_text("/retirar <objeto>", left_x, left_y, white);
    left_y += 40;

    draw_text("COMERCIO", left_x, left_y, gold);
    left_y += 30;

    draw_text("/listar", left_x, left_y, white);
    left_y += 22;
    draw_text("/comprar <nombre>", left_x, left_y, white);
    left_y += 22;
    draw_text("/vender <nombre>", left_x, left_y, white);
    left_y += 40;

    draw_text("CHAT PRIVADO", left_x, left_y, gold);
    left_y += 30;

    draw_text("@Jugador mensaje", left_x, left_y, white);
    left_y += 40;

    draw_text("ESC para cerrar", left_x, wh + wy - 40, {150, 255, 150, 255});

    // DERECHA

    draw_text("CLANES", right_x, right_y, gold);
    right_y += 30;

    draw_text("/fundar-clan <nombre>", right_x, right_y, white);
    right_y += 22;
    draw_text("/unirse <clan>", right_x, right_y, white);
    right_y += 22;
    draw_text("/revisar-clan", right_x, right_y, white);
    right_y += 22;
    draw_text("/clan-aceptar <jugador>", right_x, right_y, white);
    right_y += 22;
    draw_text("/clan-rechazar <jugador>", right_x, right_y, white);
    right_y += 22;
    draw_text("/clan-ban <jugador>", right_x, right_y, white);
    right_y += 22;
    draw_text("/clan-kick <jugador>", right_x, right_y, white);
    right_y += 22;
    draw_text("/dejar-clan", right_x, right_y, white);
    right_y += 40;

    draw_text("ATAJOS", right_x, right_y, gold);
    right_y += 30;

    draw_text("TAB -> Inventario", right_x, right_y, white);
    right_y += 22;
    draw_text("H   -> Ayuda", right_x, right_y, white);
    right_y += 22;
    draw_text("M   -> Meditar", right_x, right_y, white);
    right_y += 22;
    draw_text("R   -> Resucitar", right_x, right_y, white);
    right_y += 22;
    draw_text("E   -> Recoger item", right_x, right_y, white);
    right_y += 22;
    draw_text("Q   -> Tirar item", right_x, right_y, white);
    right_y += 22;
    draw_text("P   -> Usar pocion", right_x, right_y, white);
    right_y += 22;
    draw_text("1-2-3 -> Hechizos", right_x, right_y, white);
    right_y += 40;

    draw_text("CHEATS (Ctrl+Shift)", right_x, right_y, {255, 120, 120, 255});
    right_y += 30;

    draw_text("H -> Vida infinita", right_x, right_y, white);
    right_y += 22;
    draw_text("N -> Mana infinito", right_x, right_y, white);
    right_y += 22;
    draw_text("K -> Muerte instantanea", right_x, right_y, white);
    right_y += 22;
    draw_text("R -> Revivir instantaneamente", right_x, right_y, white);
}
