#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <atomic>
#include <memory>
#include <string>

#include "../game/PlayerState.h"
#include "../game/ChatWidget.h"
#include "../game/StatsPanel.h"
#include "../game/InventoryPanel.h"
#include "../game/PositionLabel.h"
#include "../render/Camera.h"
#include "../audio/AudioManager.h"
#include "../audio/GameAudioService.h"
#include "../../common/queue.h"
#include "../../common/protocol/dtos.h"
#include "../../common/MapaDTO.h"
#include "../net/Command.h"
#include "WorldState.h"
#include "SnapshotProcessor.h"
#include "WorldRenderer.h"
#include "InputController.h"
#include "PlayerActionController.h"

// Orquestador del juego: corre el loop principal y delega todo el trabajo
// pesado a colaboradores con responsabilidad única —
//   InputController        : eventos SDL + atajos de teclado + movimiento
//   PlayerActionController : qué pasa al clickear el mundo / comandos de chat
//   SnapshotProcessor       : interpreta los snapshots del servidor
//   WorldRenderer           : dibuja el mundo
//   GameAudioService        : único punto de acceso a sonido
// GameLoop sólo los construye, los conecta entre sí, y corre run()/update()/render().
class GameLoop {
public:
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer);

    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
             Queue<Command>* command_queue,
             Queue<SnapshotDTO>* snapshot_queue,
             Queue<MapaDTO>* map_queue,
             std::atomic<bool>* connected,
             AudioManager* audio = nullptr);

    void run();
    void stop();

private:
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
              Queue<Command>* command_queue, Queue<SnapshotDTO>* snapshot_queue,
              Queue<MapaDTO>* map_queue, std::atomic<bool>* connected,
              AudioManager* audio, const std::string& welcome_message);

    void update(float dt);
    void render();

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

    std::unique_ptr<ChatWidget>     _chat;
    std::unique_ptr<StatsPanel>     _stats;
    std::unique_ptr<InventoryPanel> _inventory;
    std::unique_ptr<PositionLabel>  _pos_label;

    GameAudioService       _audio_service;
    SnapshotProcessor       _snapshot_processor;
    WorldRenderer           _world_renderer;
    InputController         _input;
    PlayerActionController  _actions;
};
