// Patrón aplicado: Command + tabla de dispatch para el switch de 11
// InputAction en handle_keydown (ver action_table()). El switch de 4
// cheats por Ctrl+Shift+tecla se deja como if/else: son pocos, estables,
// y van gateados por un combo de modificadores que no comparten con el
// resto de los atajos, así que meterlos en la misma tabla complicaría
// más de lo que simplifica.
#include "InputController.h"

#include "../config/ClientConfig.h"
#include "../ui/ChatWidget.h"
#include "../ui/InventoryPanel.h"
#include "../ui/PositionLabel.h"
#include "../ui/StatsPanel.h"

InputController::InputController(Camera& camera, PlayerState& player, Queue<Command>* command_queue,
                                 ChatWidget* chat, StatsPanel* stats, InventoryPanel* inventory,
                                 PositionLabel* pos_label):
        _camera(camera),
        _player(player),
        _command_queue(command_queue),
        _chat(chat),
        _stats(stats),
        _inventory(inventory),
        _pos_label(pos_label) {
    const auto& kb = ClientConfig::instance().keybindings;
    _key_actions[kb.toggle_position_label] = InputAction::ToggleNameTag;
    _key_actions[kb.toggle_inventory] = InputAction::ToggleInventory;
    _key_actions[kb.drop_item] = InputAction::DropItem;
    _key_actions[kb.meditate] = InputAction::Meditate;
    _key_actions[kb.resurrect] = InputAction::Resurrect;
    _key_actions[kb.pick_item] = InputAction::PickItem;
    _key_actions[kb.quit] = InputAction::Quit;
    _key_actions[kb.help] = InputAction::Help;
    // Atajos de hechizo: 1, 2, 3
    _key_actions[SDLK_1] = InputAction::Spell1;
    _key_actions[SDLK_2] = InputAction::Spell2;
    _key_actions[SDLK_3] = InputAction::Spell3;
    // Atajo de poción: P
    _key_actions[SDLK_p] = InputAction::UsePotion;
}

void InputController::handle_events(bool& running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            continue;
        }

        // El inventario tiene prioridad (modal)
        if (_inventory && _inventory->is_visible()) {
            if (_inventory->handle_event(event, _command_queue))
                continue;
        }

        // Panel de stats (botón inventario + hechizos)
        if (_stats && _stats->handle_event(event)) {
            if (_stats->inventory_button_clicked() && _inventory)
                _inventory->toggle();
            continue;
        }

        if (_chat && _chat->handle_event(event))
            continue;

        if (event.type == SDL_KEYDOWN) {
            handle_keydown(event, running);
            continue;
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            _camera.set_screen_size(event.window.data1, event.window.data2, StatsPanel::PANEL_W);
        } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            handle_mouse_click(event.button.x, event.button.y);
        }
    }
}

void InputController::handle_keydown(const SDL_Event& event, bool& running) {
    bool inventory_open = _inventory && _inventory->is_visible();
    bool chat_active = _chat && _chat->input_active();

    // Cheats (Ctrl+Shift+tecla): vida infinita, mana infinito, morir/revivir
    // automáticamente. Se ignoran mientras se escribe en el chat para no
    // disparar por accidente con esas combinaciones.
    if (!chat_active) {
        Uint16 mods = event.key.keysym.mod;
        if ((mods & KMOD_CTRL) && (mods & KMOD_SHIFT) && _command_queue) {
            switch (event.key.keysym.sym) {
                case SDLK_h:
                    _command_queue->push(Command::cheat(CheatId::INFINITE_HP));
                    return;
                case SDLK_n:
                    _command_queue->push(Command::cheat(CheatId::INFINITE_MP));
                    return;
                case SDLK_k:
                    _command_queue->push(Command::cheat(CheatId::INSTANT_DEATH));
                    return;
                case SDLK_r:
                    _command_queue->push(Command::cheat(CheatId::INSTANT_REVIVE));
                    return;
                default:
                    break;
            }
        }
    }

    auto it = _key_actions.find(event.key.keysym.sym);
    if (it == _key_actions.end())
        return;

    const auto& table = action_table();
    auto handler = table.find(it->second);
    if (handler != table.end())
        handler->second(*this, inventory_open, chat_active, running);
}

const std::unordered_map<InputAction, InputController::ActionHandler>&
        InputController::action_table() {
    static const std::unordered_map<InputAction, ActionHandler> table = {
            {InputAction::ToggleNameTag,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._pos_label)
                     s._pos_label->toggle_visibility();
             }},
            {InputAction::ToggleInventory,
             [](InputController& s, bool, bool chat, bool&) {
                 if (!chat && s._inventory)
                     s._inventory->toggle();
             }},
            {InputAction::DropItem,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (inv && !chat && s._command_queue) {
                     int slot = s._inventory->selected_slot();
                     if (slot >= 0)
                         s._command_queue->push(Command::drop(static_cast<uint8_t>(slot)));
                 }
             }},
            {InputAction::Meditate,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._command_queue)
                     s._command_queue->push(Command::meditate());
             }},
            {InputAction::Resurrect,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._command_queue) {
                     s._command_queue->push(Command::resurrect());
                     if (s._on_resurrect)
                         s._on_resurrect();
                 }
             }},
            {InputAction::PickItem,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._command_queue)
                     s._command_queue->push(Command::pick_item());
             }},
            {InputAction::Quit,
             [](InputController&, bool, bool, bool& running) { running = false; }},
            // ─── Atajos de hechizo ───
            {InputAction::Spell1,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._stats)
                     s._stats->activate_spell_by_index(0);
             }},
            {InputAction::Spell2,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._stats)
                     s._stats->activate_spell_by_index(1);
             }},
            {InputAction::Spell3,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._stats)
                     s._stats->activate_spell_by_index(2);
             }},
            // ─── Atajo de poción ───
            {InputAction::UsePotion,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._on_use_potion)
                     s._on_use_potion();
             }},
            // ─── Atajo de ayuda ───
            {InputAction::Help,
             [](InputController& s, bool inv, bool chat, bool&) {
                 if (!inv && !chat && s._stats)
                     s._stats->toggle_help();
             }},
    };
    return table;
}

void InputController::handle_mouse_click(int mouse_x, int mouse_y) {
    if (!_command_queue || !_on_world_click)
        return;
    if (mouse_x >= _camera.game_area_w())
        return;

    int world_x = mouse_x - _camera.tile_to_screen_x(0);
    int world_y = mouse_y - _camera.tile_to_screen_y(0);
    int tile_x = world_x / ClientConfig::instance().tile_size();
    int tile_y = world_y / ClientConfig::instance().tile_size();
    _on_world_click(tile_x, tile_y);
}

void InputController::handle_movement() {
    if (_inventory && _inventory->is_visible())
        return;
    if (_chat && _chat->input_active())
        return;
    if (!_command_queue && _player.is_moving())
        return;

    Uint32 now = SDL_GetTicks();
    if (_command_queue && (now - _last_move_tick) < 200)
        return;

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    const auto& kb = ClientConfig::instance().keybindings;

    auto any_pressed = [&](const std::vector<SDL_Scancode>& scancodes) {
        for (SDL_Scancode sc: scancodes)
            if (keys[sc])
                return true;
        return false;
    };

    int dx = 0, dy = 0;
    Direction dir = _player.direction;

    if (any_pressed(kb.move_up)) {
        dy = -1;
        dir = Direction::NORTH;
    } else if (any_pressed(kb.move_down)) {
        dy = +1;
        dir = Direction::SOUTH;
    } else if (any_pressed(kb.move_left)) {
        dx = -1;
        dir = Direction::WEST;
    } else if (any_pressed(kb.move_right)) {
        dx = +1;
        dir = Direction::EAST;
    }

    if (dx == 0 && dy == 0)
        return;

    int new_x = _player.tile_x + dx;
    int new_y = _player.tile_y + dy;

    if (_command_queue) {
        _last_move_tick = now;
        _command_queue->push(
                Command::move(static_cast<uint16_t>(new_x), static_cast<uint16_t>(new_y)));
    } else {
        _player.move_to(new_x, new_y, dir);
    }
}
