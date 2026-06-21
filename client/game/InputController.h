#pragma once

#include <SDL2/SDL.h>
#include <functional>
#include <unordered_map>

#include "../net/Command.h"
#include "../../common/queue.h"
#include "../render/Camera.h"
#include "PlayerState.h"

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
    Quit,
};

// Bucle de eventos SDL + polling de movimiento continuo. Construye, una sola
// vez, una tabla `SDL_Keycode -> InputAction` a partir de los keybindings
// configurados, así `handle_events()` resuelve atajos con un lookup + switch
// en lugar de una cascada de comparaciones contra SDLK_* literales.
class InputController {
public:
    InputController(Camera& camera, PlayerState& player, Queue<Command>* command_queue,
                     ChatWidget* chat, StatsPanel* stats, InventoryPanel* inventory,
                     PositionLabel* pos_label);

    // Procesa los eventos SDL pendientes. Pone running en false si corresponde salir.
    void handle_events(bool& running);

    // Polling de movimiento continuo (WASD/flechas configurables). Con cola
    // de comandos empuja un Command::move; sin ella (modo offline/preview)
    // mueve PlayerState directamente.
    void handle_movement();

    // Callback invocado con la tile (en coordenadas de mundo) clickeada con
    // el botón izquierdo dentro del área de juego.
    void on_world_click(std::function<void(int tile_x, int tile_y)> cb) { _on_world_click = std::move(cb); }

private:
    void handle_keydown(const SDL_Event& event, bool& running);
    void handle_mouse_click(int mouse_x, int mouse_y);

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
};
