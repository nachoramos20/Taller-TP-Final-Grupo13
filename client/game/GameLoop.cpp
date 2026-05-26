#include "GameLoop.h"
#include <algorithm>

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer)
    : _window(window), _renderer(renderer),
      _camera(window.GetWidth(), window.GetHeight()),
      _running(false),
      _command_queue(nullptr),
      _snapshot_queue(nullptr),
      _connected(nullptr) {}

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
                   Queue<Command>* command_queue,
                   Queue<SnapshotDTO>* snapshot_queue,
                   std::atomic<bool>* connected)
    : _window(window), _renderer(renderer),
      _camera(window.GetWidth(), window.GetHeight()),
      _running(false),
      _command_queue(command_queue),
      _snapshot_queue(snapshot_queue),
      _connected(connected) {}

void GameLoop::run() {
    _running = true;
    Uint32 prev_ticks = SDL_GetTicks();

    while (_running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - prev_ticks) / 1000.0f;
        prev_ticks = now;

        handle_events();
        handle_input();
        update(dt);
        render();
    }
}

void GameLoop::stop() { _running = false; }

void GameLoop::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            _running = false;
        } else if (event.type == SDL_KEYDOWN
                   && event.key.keysym.sym == SDLK_ESCAPE) {
            _running = false;
        } else if (event.type == SDL_WINDOWEVENT
                   && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            _camera.set_screen_size(event.window.data1, event.window.data2);
        }
    }
}

void GameLoop::handle_input() {
    if (!_command_queue && _player.is_moving()) return;

    // Limitar a 1 movimiento cada 250ms
    Uint32 now = SDL_GetTicks();
    if (_command_queue && (now - _last_move_tick) < 250) return;

    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    int dx = 0, dy = 0;
    Direction dir = _player.direction;

    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
        dy = -1; dir = Direction::NORTH;
    } else if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        dy = +1; dir = Direction::SOUTH;
    } else if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        dx = -1; dir = Direction::WEST;
    } else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        dx = +1; dir = Direction::EAST;
    }

    if (dx != 0 || dy != 0) {
        int new_x = _player.tile_x + dx;
        int new_y = _player.tile_y + dy;
        if (new_x >= 0 && new_x < MAP_SIZE &&
            new_y >= 0 && new_y < MAP_SIZE) {
            if (_command_queue) {
                _last_move_tick = now;
                _command_queue->push(Command::move(
                    static_cast<uint16_t>(new_x),
                    static_cast<uint16_t>(new_y)
                ));
            } else {
                _player.move_to(new_x, new_y, dir);
            }
        }
    }
}

void GameLoop::update(float dt) {
    if (_connected && !(*_connected)) {
        _running = false;
        return;
    }

    if (_snapshot_queue) {
        SnapshotDTO snap;
        while (_snapshot_queue->try_pop(snap)) {
            apply_snapshot(snap);
        }
    }

    _player.update(dt);
    _camera.follow(_player);
}

void GameLoop::apply_snapshot(const SnapshotDTO& snap) {
    _last_entities = snap.entities;
    _my_entity_id  = snap.self_entity_id;

    for (const auto& e : snap.entities) {
        if (e.entity_id == snap.self_entity_id) {
            Direction dir = static_cast<Direction>(e.direction);
            _player.move_to(e.pos_x, e.pos_y, dir);
            break;
        }
    }
}

void GameLoop::render() {
    _renderer.SetDrawColor(30, 30, 30, 255);
    _renderer.Clear();
    render_map();
    render_entities();
    _renderer.Present();
}

void GameLoop::render_map() {
    int screen_w = _window.GetWidth();
    int screen_h = _window.GetHeight();

    int first_x = std::max(0, -_camera.tile_to_screen_x(0) / TILE_SIZE);
    int first_y = std::max(0, -_camera.tile_to_screen_y(0) / TILE_SIZE);
    int last_x  = std::min(MAP_SIZE - 1, first_x + screen_w / TILE_SIZE + 2);
    int last_y  = std::min(MAP_SIZE - 1, first_y + screen_h / TILE_SIZE + 2);

    for (int ty = first_y; ty <= last_y; ty++) {
        for (int tx = first_x; tx <= last_x; tx++) {
            int sx = _camera.tile_to_screen_x(tx);
            int sy = _camera.tile_to_screen_y(ty);
            _renderer.SetDrawColor(34, 85, 34, 255);
            _renderer.FillRect(SDL2pp::Rect(sx, sy, TILE_SIZE, TILE_SIZE));
            _renderer.SetDrawColor(20, 60, 20, 255);
            _renderer.DrawRect(SDL2pp::Rect(sx, sy, TILE_SIZE, TILE_SIZE));
        }
    }
}

void GameLoop::render_entities() {
    for (const auto& e : _last_entities) {
        int world_px = e.pos_x * TILE_SIZE;
        int world_py = e.pos_y * TILE_SIZE;

        // Si es el jugador propio, usar posición interpolada
        if (e.entity_id == _my_entity_id) {
            world_px = static_cast<int>(_player.pixel_x());
            world_py = static_cast<int>(_player.pixel_y());
        }

        int sx = _camera.world_to_screen_x(static_cast<float>(world_px));
        int sy = _camera.world_to_screen_y(static_cast<float>(world_py));

        int offset = (TILE_SIZE - 24) / 2;

        // Propio: rojo, otros: azul
        if (e.entity_id == _my_entity_id) {
            _renderer.SetDrawColor(220, 60, 60, 255);
        } else {
            _renderer.SetDrawColor(60, 60, 220, 255);
        }
        _renderer.FillRect(SDL2pp::Rect(sx + offset, sy + offset, 24, 24));

        // Indicador de dirección
        int cx = sx + TILE_SIZE / 2;
        int cy = sy + TILE_SIZE / 2;
        int dx = 0, dy = 0;
        switch (static_cast<Direction>(e.direction)) {
            case Direction::NORTH: dy = -10; break;
            case Direction::SOUTH: dy = +10; break;
            case Direction::EAST:  dx = +10; break;
            case Direction::WEST:  dx = -10; break;
            default: break;
        }
        _renderer.SetDrawColor(255, 255, 255, 255);
        _renderer.DrawLine(cx, cy, cx + dx, cy + dy);
    }
}