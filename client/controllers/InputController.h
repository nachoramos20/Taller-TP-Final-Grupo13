#pragma once

#include <SDL2/SDL.h>
#include <functional>
#include <unordered_map>

#include "../net/Command.h"
#include "../../common/queue.h"
#include "../render/Camera.h"
#include "../PlayerState.h"

class ChatWidget;
class StatsPanel;
class InventoryPanel;
class PositionLabel;

// Atajos de teclado disponibles. La tecla de cada uno se resuelve desde
// ClientConfig::Keybindings (cargado del .toml), no está hardcodeada acá.
enum class InputAction {
    ToggleNameTag,
    ToggleInventory,
    DropItem,
    Meditate,
    Resurrect,
    PickItem,
    Spell1,
    Spell2,
    Spell3,
    UsePotion,
    Quit,
    Help,
};

// Bucle de eventos SDL + polling de movimiento continuo.
class InputController {
public:
    InputController(Camera& camera, PlayerState& player, Queue<Command>* command_queue,
                     ChatWidget* chat, StatsPanel* stats, InventoryPanel* inventory,
                     PositionLabel* pos_label);

    void handle_events(bool& running);
    void handle_movement();

    void on_world_click(std::function<void(int tile_x, int tile_y)> cb) { _on_world_click = std::move(cb); }
    // Callback para "usar poción" desde atajo de teclado
    void on_use_potion(std::function<void()> cb) { _on_use_potion = std::move(cb); }
    // Callback para "resucitar" desde atajo de teclado (sonido del sacerdote)
    void on_resurrect(std::function<void()> cb) { _on_resurrect = std::move(cb); }

private:
    void handle_keydown(const SDL_Event& event, bool& running);
    void handle_mouse_click(int mouse_x, int mouse_y);

    // Tabla de dispatch por InputAction (Command pattern, en vez del switch
    // de 11 casos en handle_keydown): cada entrada ejecuta la acción de un
    // atajo ya resuelto por _key_actions. `inventory_open`/`chat_active` se
    // pasan calculados porque casi todas las acciones los usan para
    // bloquearse mientras el inventario o el chat están activos.
    using ActionHandler = std::function<void(InputController&, bool inventory_open, bool chat_active, bool& running)>;
    static const std::unordered_map<InputAction, ActionHandler>& action_table();

    Camera&         _camera;
    PlayerState&    _player;
    Queue<Command>* _command_queue;
    ChatWidget*     _chat;
    StatsPanel*     _stats;
    InventoryPanel* _inventory;
    PositionLabel*  _pos_label;

    std::unordered_map<SDL_Keycode, InputAction> _key_actions;
    Uint32 _last_move_tick = 0;

    std::function<void(int, int)> _on_world_click;
    std::function<void()>         _on_use_potion;
    std::function<void()>         _on_resurrect;
};
