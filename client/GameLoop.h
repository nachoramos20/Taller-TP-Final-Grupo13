#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <atomic>
#include <memory>
#include <string>

#include "PlayerState.h"
#include "ui/HudManager.h"
#include "render/Camera.h"
#include "audio/AudioManager.h"
#include "audio/GameAudioService.h"
#include "../common/queue.h"
#include "../common/protocol/dtos.h"
#include "../common/protocol/MapaDTO.h"
#include "net/Command.h"
#include "WorldState.h"
#include "processors/SnapshotProcessor.h"
#include "render/WorldRenderer.h"
#include "controllers/InputController.h"
#include "controllers/PlayerActionController.h"

// El bucle principal del juego ya autenticado: por cada vuelta procesa
// input, aplica los snapshots/mapas que llegaron del servidor, actualiza
// el estado local (cámara, animaciones, sonido ambiente) y renderiza.
class GameLoop {
public:
    // Sin sesión de red (colas y audio en null): construcción mínima para
    // los casos en que no hace falta procesar mensajes del servidor.
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer);

    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
             Queue<Command>* command_queue,
             Queue<SnapshotDTO>* snapshot_queue,
             Queue<MapaDTO>* map_queue,
             std::atomic<bool>* connected,
             AudioManager* audio = nullptr,
             const std::string& username = "");

    void run();
    void stop();

private:
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
              Queue<Command>* command_queue, Queue<SnapshotDTO>* snapshot_queue,
              Queue<MapaDTO>* map_queue, std::atomic<bool>* connected,
              AudioManager* audio, const std::string& welcome_message,
              const std::string& username);

    void update(float dt);
    void render();
    void handle_use_potion();

    SDL2pp::Window&     _window;
    SDL2pp::Renderer&   _renderer;
    Camera              _camera;
    PlayerState         _player;
    bool                _running = false;

    Queue<Command>*      _command_queue;
    Queue<SnapshotDTO>*  _snapshot_queue;
    Queue<MapaDTO>*      _map_queue;
    std::atomic<bool>*   _connected;
    AudioManager*        _audio;

    WorldState _state;

    HudManager _hud;

    GameAudioService       _audio_service;
    SnapshotProcessor       _snapshot_processor;
    WorldRenderer           _world_renderer;
    InputController         _input;
    PlayerActionController  _actions;
};
