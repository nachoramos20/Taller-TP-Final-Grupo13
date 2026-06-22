#include "GameLoop.h"
#include "../config/ClientConfig.h"
#include "../config/RacesClassesConfig.h"
#include "../../common/protocol/protocol.h"

// Constructores

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer)
    : GameLoop(window, renderer, nullptr, nullptr, nullptr, nullptr, nullptr,
               RacesClassesConfig::instance().get_login_messages().welcome, "") {}

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
                   Queue<Command>* command_queue,
                   Queue<SnapshotDTO>* snapshot_queue,
                   Queue<MapaDTO>* map_queue,
                   std::atomic<bool>* connected,
                   AudioManager* audio,
                   const std::string& username)
    : GameLoop(window, renderer, command_queue, snapshot_queue, map_queue, connected, audio,
               RacesClassesConfig::instance().get_login_messages().connected, username) {}

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
                    Queue<Command>* command_queue, Queue<SnapshotDTO>* snapshot_queue,
                    Queue<MapaDTO>* map_queue, std::atomic<bool>* connected,
                    AudioManager* audio, const std::string& welcome_message,
                    const std::string& username)
    : _window(window), _renderer(renderer),
      _camera(window.GetWidth(), window.GetHeight(), StatsPanel::PANEL_W),
      _command_queue(command_queue), _snapshot_queue(snapshot_queue),
      _map_queue(map_queue), _connected(connected), _audio(audio),
      _chat(std::make_unique<ChatWidget>(renderer, ClientConfig::instance().fonts.chat_font_path,
                                          ClientConfig::instance().fonts.chat_font_size)),
      _stats(std::make_unique<StatsPanel>(renderer, ClientConfig::instance().fonts.chat_font_path,
                                           ClientConfig::instance().fonts.medium_font_size,
                                           &_audio_service)),
      _inventory(std::make_unique<InventoryPanel>(renderer, ClientConfig::instance().fonts.chat_font_path,
                                                   ClientConfig::instance().fonts.small_font_size,
                                                   &_audio_service)),
      _pos_label(std::make_unique<PositionLabel>(renderer, ClientConfig::instance().fonts.chat_font_path,
                                                  ClientConfig::instance().fonts.medium_font_size)),
      _audio_service(audio),
      _snapshot_processor(_audio_service, _stats.get(), _inventory.get(), _chat.get()),
      _world_renderer(window, renderer, _camera),
      _input(_camera, _player, _command_queue, _chat.get(), _stats.get(), _inventory.get(), _pos_label.get()),
      _actions(_state, _player, _command_queue, _chat.get(), _stats.get(), _audio_service) {

    if (!username.empty())
        _stats->set_username(username);

    _chat->add_message(welcome_message);

    _input.on_world_click([this](int tile_x, int tile_y) {
        _actions.handle_world_click(tile_x, tile_y);
    });

    _input.on_use_potion([this]() {
        handle_use_potion();
    });

    _chat->on_submit([this](const std::string& text) {
        if (!_command_queue) return;
        _command_queue->push(Command::chat(text));
        _chat->add_message("> " + text);
        _actions.handle_chat_command(text);
    });

    _world_renderer.load_item_textures(*_inventory);
}

// run / stop

void GameLoop::run() {
    _running = true;
    Uint32 prev_ticks = SDL_GetTicks();

    while (_running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - prev_ticks) / 1000.0f;
        prev_ticks = now;

        _input.handle_events(_running);
        _input.handle_movement();
        update(dt);
        render();
    }
}

void GameLoop::stop() { _running = false; }

// update

void GameLoop::update(float dt) {
    if (_connected && !(*_connected)) {
        _running = false;
        return;
    }

    if (_map_queue) {
        MapaDTO map;
        if (_map_queue->try_pop(map))
            _snapshot_processor.apply_map(_state, map);
    }

    if (_snapshot_queue) {
        SnapshotDTO snap;
        while (_snapshot_queue->try_pop(snap))
            _snapshot_processor.apply_snapshot(_state, _player, snap);
    }

    // Mantener la referencia al inventario actualizada en StatsPanel (para atajo poción)
    _stats->set_inventory_ref(_state.inventory, SnapshotDTO::INVENTORY_SIZE);

    _player.update(dt);
    advance_entity_motion(_state, dt);
    _camera.follow(_player);
    _pos_label->update(_player.tile_x, _player.tile_y);
    if (_audio) {
        _audio->update();
        _audio_service.update_ocean_ambient(distance_to_nearest_water_tile(_state, _player));
        bool walking_on_city_stone = _player.is_moving()
            && is_floor_city_stone(_state, static_cast<uint16_t>(_player.tile_x),
                                    static_cast<uint16_t>(_player.tile_y));
        _audio_service.update_city_stone_footsteps(walking_on_city_stone);
        _audio_service.update_forest_ambience(is_in_forest_zone(
            static_cast<uint16_t>(_player.tile_x), static_cast<uint16_t>(_player.tile_y)));
        _audio_service.update_cemetery_ambient(distance_to_cemetery_zone(_player.tile_x, _player.tile_y));
    }
}

// render

void GameLoop::render() {
    _renderer.SetDrawColor(0, 0, 0, 255);
    _renderer.Clear();

    _world_renderer.render(_state, _player);

    int sw = _window.GetWidth();
    int sh = _window.GetHeight();
    if (_chat)      _chat->render(sw, sh);
    if (_stats)     _stats->render(sw, sh);
    if (_inventory) _inventory->render(sw, sh);
    if (_pos_label) _pos_label->render(sw, sh);

    _renderer.Present();
}

// Usar la primera poción de salud o maná que encuentre en el inventario
void GameLoop::handle_use_potion() {
    if (!_command_queue) return;

    const uint8_t HEALTH_POTION = static_cast<uint8_t>(ItemId::HEALTH_POTION);
    const uint8_t MANA_POTION   = static_cast<uint8_t>(ItemId::MANA_POTION);

    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++) {
        uint8_t item = _state.inventory[i];
        if (item == HEALTH_POTION || item == MANA_POTION) {
            _command_queue->push(Command::use_item(static_cast<uint8_t>(i)));
            if (_chat) _chat->add_message("Usaste una poción.");
            return;
        }
    }
    if (_chat) _chat->add_message("No tienes pociones en el inventario.");
}
