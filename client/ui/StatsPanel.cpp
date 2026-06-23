#include "StatsPanel.h"

#include <stdexcept>
#include <string>

#include "../../common/protocol/WeaponRules.h"
#include "../../common/protocol/protocol.h"
#include "../audio/GameAudioService.h"

StatsPanel::StatsPanel(SDL2pp::Renderer& renderer, const std::string& font_path, int font_size,
                       GameAudioService* audio):
        _renderer(renderer), _audio(audio), _font_size(font_size) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0)
            throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    _font = TTF_OpenFont(font_path.c_str(), font_size);
    if (!_font)
        throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());

    _render_impl.emplace(_renderer, _font, _font_size);
}

StatsPanel::~StatsPanel() {
    if (_font)
        TTF_CloseFont(_font);
}

void StatsPanel::set_inventory_ref(const uint8_t* inv, int size) {
    _inv_ref = inv;
    _inv_size = size;
}

void StatsPanel::update(uint16_t hp, uint16_t max_hp, uint16_t mp, uint16_t max_mp, uint32_t gold,
                        uint8_t level, uint32_t exp, bool meditating, bool is_ghost, uint8_t cls,
                        uint8_t equipped_weapon_item_id) {
    _hp = hp;
    _max_hp = std::max<uint16_t>(1, max_hp);
    _mp = mp;
    _max_mp = max_mp;
    _gold = gold;
    _level = level;
    _exp = exp;
    _meditating = meditating;
    _is_ghost = is_ghost;
    _cls = cls;
    _eq_weapon_item = equipped_weapon_item_id;

    _spell_system.disable_if_weapon_cant_cast(_eq_weapon_item);
}

void StatsPanel::activate_spell_by_index(int index) {
    auto spells = _spell_system.spells_for_class(_cls);
    if (index < 0 || index >= (int)spells.size())
        return;
    if (_audio)
        _audio->click();
    _spell_system.activate_by_index(_cls, index);
}

bool StatsPanel::handle_event(const SDL_Event& e) {
    _inv_clicked = false;

    // Cerrar popup de ayuda
    if (_help_visible && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {

        _help_visible = false;
        return true;
    }

    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT)
        return false;

    int mx = e.button.x, my = e.button.y;

    auto in = [&](const SDL_Rect& r) {
        return r.w > 0 && mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
    };

    if (in(_layout.inv_btn_rect)) {
        _inv_clicked = true;
        return true;
    }

    if (in(_layout.help_btn_rect)) {
        if (_audio)
            _audio->click();

        _help_visible = !_help_visible;
        return true;
    }

    for (int i = 0; i < _layout.spell_btn_count; i++) {
        if (in(_layout.spell_btn_rect[i])) {
            if (_audio)
                _audio->click();

            _spell_system.toggle(_layout.spell_btn_id[i]);
            return true;
        }
    }

    return false;
}

void StatsPanel::render(int screen_w, int screen_h) {
    auto spells = _spell_system.spells_for_class(_cls);

    StatsRenderInput input;
    input.username = &_username;
    input.hp = _hp;
    input.max_hp = _max_hp;
    input.mp = _mp;
    input.max_mp = _max_mp;
    input.gold = _gold;
    input.exp = _exp;
    input.level = _level;
    input.meditating = _meditating;
    input.is_ghost = _is_ghost;
    input.cls = _cls;
    input.eq_weapon_item = _eq_weapon_item;
    input.help_visible = _help_visible;
    input.cast_mode = _spell_system.cast_mode_active();
    input.selected_spell = _spell_system.selected_spell();
    input.spells = &spells;

    _layout = _render_impl->render(screen_w, screen_h, input);
}
