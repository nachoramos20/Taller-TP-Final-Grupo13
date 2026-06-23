#include "InventoryPanel.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "../audio/GameAudioService.h"

InventoryPanel::InventoryPanel(SDL2pp::Renderer& renderer, const std::string& font_path,
                               int font_size, GameAudioService* audio):
        _renderer(renderer), _audio(audio), _font_size(font_size) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0)
            throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    _font = TTF_OpenFont(font_path.c_str(), font_size);
    if (!_font)
        throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());

    _font_sm = TTF_OpenFont(font_path.c_str(), std::max(8, font_size - 2));

    std::memset(_inventory, 0, sizeof(_inventory));

    _render_impl.emplace(_renderer, _font, _font_sm, _font_size);
}

InventoryPanel::~InventoryPanel() {
    if (_font)
        TTF_CloseFont(_font);
    if (_font_sm)
        TTF_CloseFont(_font_sm);
}

void InventoryPanel::update(const uint8_t inventory[INV_SIZE], uint8_t equipped_weapon_slot,
                            uint8_t equipped_armor_slot, uint8_t equipped_helmet_slot,
                            uint8_t equipped_shield_slot) {
    std::memcpy(_inventory, inventory, INV_SIZE);
    _eq_wpn = equipped_weapon_slot;
    _eq_arm = equipped_armor_slot;
    _eq_helm = equipped_helmet_slot;
    _eq_shld = equipped_shield_slot;
}

void InventoryPanel::register_item_texture(uint8_t item_id, SDL_Texture* tex) {
    _item_textures[item_id] = tex;
}

bool InventoryPanel::handle_event(const SDL_Event& e, Queue<Command>* cmd_queue) {
    if (!_visible)
        return false;

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        _visible = false;
        return true;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;

        bool (*in)(int, int, const SDL_Rect&) = [](int mx, int my, const SDL_Rect& r) {
            return mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
        };

        if (in(mx, my, _layout.close_btn)) {
            if (_audio)
                _audio->click();
            _visible = false;
            return true;
        }

        if (_selected_slot >= 0 && _inventory[_selected_slot] != 0) {
            if (in(mx, my, _layout.equip_btn)) {
                if (_audio)
                    _audio->click();
                if (cmd_queue) {
                    uint8_t item_id = _inventory[_selected_slot];
                    bool is_potion = (item_id == 40 || item_id == 41);
                    if (is_potion) {
                        cmd_queue->push(Command::use_item(static_cast<uint8_t>(_selected_slot)));
                    } else if (_selected_slot == _eq_wpn) {
                        cmd_queue->push(Command::unequip(EquipSlot::WEAPON));
                    } else if (_selected_slot == _eq_arm) {
                        cmd_queue->push(Command::unequip(EquipSlot::ARMOR));
                    } else if (_selected_slot == _eq_helm) {
                        cmd_queue->push(Command::unequip(EquipSlot::HELMET));
                    } else if (_selected_slot == _eq_shld) {
                        cmd_queue->push(Command::unequip(EquipSlot::SHIELD));
                    } else {
                        cmd_queue->push(Command::equip(static_cast<uint8_t>(_selected_slot)));
                    }
                }
                return true;
            }
            if (in(mx, my, _layout.drop_btn)) {
                if (_audio)
                    _audio->click();
                if (cmd_queue)
                    cmd_queue->push(Command::drop(static_cast<uint8_t>(_selected_slot)));
                return true;
            }
        }

        for (int i = 0; i < INV_SIZE; i++) {
            if (in(mx, my, _layout.slot_rects[i])) {
                if (_audio)
                    _audio->click();
                _selected_slot = (_selected_slot == i) ? -1 : i;
                _drag_from_slot = i;
                return true;
            }
        }

        if (in(mx, my, _layout.panel_rect))
            return true;
    }

    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        int from = _drag_from_slot;
        _drag_from_slot = -1;
        if (from < 0 || _inventory[from] == 0)
            return false;

        int mx = e.button.x, my = e.button.y;
        bool (*in)(int, int, const SDL_Rect&) = [](int mx, int my, const SDL_Rect& r) {
            return mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
        };

        for (int i = 0; i < INV_SIZE; i++) {
            if (i != from && in(mx, my, _layout.slot_rects[i])) {
                if (_audio)
                    _audio->click();
                if (cmd_queue)
                    cmd_queue->push(Command::move_item(static_cast<uint8_t>(from),
                                                       static_cast<uint8_t>(i)));
                return true;
            }
        }
    }
    return false;
}

void InventoryPanel::render(int screen_w, int screen_h) {
    if (!_visible)
        return;

    InventoryRenderInput input;
    input.inventory = &_inventory;
    input.eq_wpn = _eq_wpn;
    input.eq_arm = _eq_arm;
    input.eq_helm = _eq_helm;
    input.eq_shld = _eq_shld;
    input.selected_slot = _selected_slot;
    input.item_textures = &_item_textures;

    _layout = _render_impl->render(screen_w, screen_h, input);
}
