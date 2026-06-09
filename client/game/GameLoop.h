#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <atomic>
#include <memory>
#include <vector>

#include "../game/PlayerState.h"
#include "../game/ChatWidget.h"
#include "../game/StatsPanel.h"
#include "../game/InventoryPanel.h"
#include "../game/PositionLabel.h"
#include "../render/Camera.h"
#include "../render/AssetManager.h"
#include "../render/AnimationSystem.h"
#include "../render/SpriteConfig.h"
#include "../render/ObjectSupConfig.h"
#include "../../common/queue.h"
#include "../../common/protocol/dtos.h"
#include "../../common/MapaDTO.h"
#include "../net/Command.h"

static constexpr int MAP_SIZE        = 100;
static constexpr int OBJ_SUP_TILES   = 6;
static constexpr int OBJ_SUP_SIZE    = OBJ_SUP_TILES * TILE_SIZE;
static constexpr int OBJ_SUP_TICKS_PER_FRAME = 8;

class GameLoop {
public:
    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer);

    GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
             Queue<Command>* command_queue,
             Queue<SnapshotDTO>* snapshot_queue,
             Queue<MapaDTO>* map_queue,
             std::atomic<bool>* connected);

    void run();
    void stop();

private:
    void handle_events();
    void handle_input();
    void handle_mouse_click(int mouse_x, int mouse_y);
    void update(float dt);
    void apply_snapshot(const SnapshotDTO& snap);
    void apply_map(const MapaDTO& map);
    void render();
    void render_floor();
    void render_objects();
    void render_entities();
    void render_obj_sup();

    SDL2pp::Window&     _window;
    SDL2pp::Renderer&   _renderer;
    Camera              _camera;
    PlayerState         _player;
    bool                _running;

    Queue<Command>*      _command_queue;
    Queue<SnapshotDTO>*  _snapshot_queue;
    Queue<MapaDTO>*      _map_queue;
    std::atomic<bool>*   _connected;

    std::vector<EntityDTO> _last_entities;
    uint16_t               _my_entity_id;
    Uint32                 _last_move_tick;
    uint32_t               _current_tick;

    MapaDTO   _map;
    bool      _map_loaded;

    AssetManager     _assets;
    AnimationSystem  _anim;
    SpriteConfig     _sprite_config;
    TileConfig       _tile_config;
    ObjectSupConfig  _obj_sup_config;

    std::unique_ptr<ChatWidget>        _chat;
    std::unique_ptr<StatsPanel>        _stats;
    std::unique_ptr<InventoryPanel>    _inventory;
    std::unique_ptr<PositionLabel>     _pos_label;
};
