#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>

#include "../game/PlayerState.h"
#include "../render/Camera.h"
#include "../../common/queue.h"
#include "../../common/protocol/dtos.h"
#include "../net/Command.h"

static constexpr int MAP_SIZE = 100;

class GameLoop {
public:
    // Sin servidor: modo local
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer);

    // Con servidor: pasa las colas
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
             Queue<Command>* command_queue,
             Queue<SnapshotDTO>* snapshot_queue);

    void run();
    void stop();

private:
    void handle_events();
    void handle_input();
    void update(float dt);
    void apply_snapshot(const SnapshotDTO& snap);
    void render();
    void render_map();
    void render_player();

    SDL2pp::Window&     _window;
    SDL2pp::Renderer&   _renderer;
    Camera              _camera;
    PlayerState         _player;
    bool                _running;

    Queue<Command>*     _command_queue;
    Queue<SnapshotDTO>* _snapshot_queue;
};