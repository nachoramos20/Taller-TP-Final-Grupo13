#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>

#include "../game/PlayerState.h"
#include "../render/Camera.h"

static constexpr int MAP_SIZE = 100;

class GameLoop {
public:
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer);

    void run();
    void stop();

private:
    void handle_events();
    void handle_input();
    void update(float dt);
    void render();
    void render_map();
    void render_player();

    SDL2pp::Window&   _window;
    SDL2pp::Renderer& _renderer;
    Camera            _camera;
    PlayerState       _player;
    bool              _running;
};