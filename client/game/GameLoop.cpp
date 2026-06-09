#include "GameLoop.h"
#include <algorithm>
#include <unordered_map>
#include <string>
#include "../../common/protocol/protocol.h"

static const char* CHAT_FONT_PATH = "assets/fonts/DejaVuSans.ttf";

// Constructores

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer)
    : _window(window), _renderer(renderer),
      _camera(window.GetWidth(), window.GetHeight(), StatsPanel::PANEL_W),
      _running(false),
      _command_queue(nullptr), _snapshot_queue(nullptr),
      _map_queue(nullptr), _connected(nullptr),
      _my_entity_id(0), _last_move_tick(0), _current_tick(0),
      _map_loaded(false),
      _assets(renderer),
      _sprite_config("config/sprites.toml"),
      _tile_config("config/tiles.toml", "floor"),
      _obj_sup_config("config/objects_sup.toml") {
    _anim.load();
    _chat      = std::make_unique<ChatWidget>(renderer, CHAT_FONT_PATH);
    _stats     = std::make_unique<StatsPanel>(renderer, CHAT_FONT_PATH);
    _inventory = std::make_unique<InventoryPanel>(renderer, CHAT_FONT_PATH);
    _chat->add_message("Bienvenido. Enter para chatear.");
    load_item_textures();
}

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
                   Queue<Command>* command_queue,
                   Queue<SnapshotDTO>* snapshot_queue,
                   Queue<MapaDTO>* map_queue,
                   std::atomic<bool>* connected)
    : _window(window), _renderer(renderer),
      _camera(window.GetWidth(), window.GetHeight(), StatsPanel::PANEL_W),
      _running(false),
      _command_queue(command_queue),
      _snapshot_queue(snapshot_queue),
      _map_queue(map_queue),
      _connected(connected),
      _my_entity_id(0), _last_move_tick(0), _current_tick(0),
      _map_loaded(false),
      _assets(renderer),
      _sprite_config("config/sprites.toml"),
      _tile_config("config/tiles.toml", "floor"),
      _obj_sup_config("config/objects_sup.toml") {
    _anim.load();
    _chat      = std::make_unique<ChatWidget>(renderer, CHAT_FONT_PATH);
    _stats     = std::make_unique<StatsPanel>(renderer, CHAT_FONT_PATH);
    _inventory = std::make_unique<InventoryPanel>(renderer, CHAT_FONT_PATH);

    _chat->add_message("Conectado. Enter para chatear. Click izq sobre enemigo para atacar.");
    _chat->on_submit([this](const std::string& text) {
        if (!_command_queue) return;
        _command_queue->push(Command::chat(text));
        _chat->add_message("> " + text);
    });

    load_item_textures();
}

// load_item_textures

void GameLoop::spawn_spell_effect(uint8_t spell_id, uint16_t pos_x, uint16_t pos_y) {
    struct SpellInfo { const char* path; int cols; int rows; };
    static const std::unordered_map<uint8_t, SpellInfo> spell_info = {
        { 1, { "assets/sprites/spells/explosion.png",            5, 3 }},
        { 2, { "assets/sprites/spells/area_veneno.png",          4, 4 }},
        { 3, { "assets/sprites/spells/explosion_calaverica.png", 4, 4 }},
        { 4, { "assets/sprites/spells/orbe_hielo.png",           5, 4 }},
        { 5, { "assets/sprites/spells/tornado_gravitatorio.png", 5, 2 }},
        { 6, { "assets/sprites/spells/tormenta_electrica.png",   4, 4 }},
        { 7, { "assets/sprites/spells/orbe_vacio.png",           5, 4 }},
        { 8, { "assets/sprites/spells/brecha_vacio.png",         5, 4 }},
        { 9, { "assets/sprites/spells/tornado_oscuridad.png",    5, 2 }},
    };
    auto it = spell_info.find(spell_id);
    if (it == spell_info.end()) return;

    SpellEffect fx{};
    fx.spell_id   = spell_id;
    fx.pos_x      = pos_x;
    fx.pos_y      = pos_y;
    fx.start_tick = _current_tick;
    fx.cols       = it->second.cols;
    fx.rows       = it->second.rows;
    fx.path       = it->second.path;
    _spell_effects.push_back(fx);
}

void GameLoop::render_spells() {
    // Limpiar efectos terminados
    _spell_effects.erase(
        std::remove_if(_spell_effects.begin(), _spell_effects.end(),
            [&](const SpellEffect& fx) {
                int total = fx.cols * fx.rows;
                uint32_t elapsed = _current_tick - fx.start_tick;
                return static_cast<int>(elapsed / SPELL_TICKS_PER_FRAME) >= total;
            }),
        _spell_effects.end());

    for (const auto& fx : _spell_effects) {
        int total   = fx.cols * fx.rows;
        uint32_t elapsed = _current_tick - fx.start_tick;
        int frame   = static_cast<int>(elapsed / SPELL_TICKS_PER_FRAME) % total;

        int col = frame % fx.cols;
        int row = frame / fx.cols;

        SDL2pp::Texture& tex = _assets.get(fx.path);
        int tw = tex.GetWidth()  / fx.cols;
        int th = tex.GetHeight() / fx.rows;

        SDL2pp::Rect src(col * tw, row * th, tw, th);

        // Escalar a 3×3 tiles centrado en la posición del caster
        int size_px = TILE_SIZE * 3;
        int sx = _camera.world_to_screen_x(static_cast<float>(fx.pos_x * TILE_SIZE))
                 - size_px / 2 + TILE_SIZE / 2;
        int sy = _camera.world_to_screen_y(static_cast<float>(fx.pos_y * TILE_SIZE))
                 - size_px / 2 + TILE_SIZE / 2;

        SDL2pp::Rect dst(sx, sy, size_px, size_px);
        _renderer.Copy(tex, src, dst);
    }
}

// load_item_textures

void GameLoop::load_item_textures() {
    if (!_inventory) return;

    // Mapeo ItemId → path del sprite
    static const std::vector<std::pair<uint8_t, std::string>> item_paths = {
        // Espadas
        { 1,  "assets/sprites/weapons/sword/espada_comun.png" },
        // Hachas
        { 2,  "assets/sprites/weapons/axe/hacha_hierro.png" },
        // Martillos
        { 3,  "assets/sprites/weapons/hammer/martillo_comun.png" },
        // Arcos
        { 4,  "assets/sprites/weapons/bow/arco_simple_madera.png" },
        { 5,  "assets/sprites/weapons/bow/arco_compuesto_oro.png" },
        // Armas mágicas
        { 6,  "assets/sprites/weapons/flute/flauta_elfica.png" },
        { 7,  "assets/sprites/weapons/staff/baculo_esmeralda.png" },
        { 8,  "assets/sprites/weapons/stick/vara_fresno.png" },
        // Armaduras
        { 10, "assets/sprites/equipment/armor/guerrero_ejecutor.png" },
        { 11, "assets/sprites/equipment/armor/paladin_real.png" },
        // Cascos
        { 20, "assets/sprites/equipment/helmet/cascos.png" },
        { 21, "assets/sprites/equipment/helmet/cascos.png" },
        { 22, "assets/sprites/equipment/helmet/cascos.png" },
        // Escudos
        { 30, "assets/sprites/equipment/shield/escudo_tortuga.png" },
        { 31, "assets/sprites/equipment/shield/escudo_hierro.png" },
        // Pociones
        { 40, "assets/sprites/items/pocion_vida.png" },
        { 41, "assets/sprites/items/pocion_mana.png" },
        // Oro
        { 50, "assets/sprites/items/oro.png" },
    };

    for (const auto& [item_id, path] : item_paths) {
        try {
            SDL2pp::Texture& tex = _assets.get(path);
            _inventory->register_item_texture(item_id, tex.Get());
        } catch (...) {
            // Si el asset no existe, el slot mostrará la abreviatura en texto
        }
    }
}

// run / stop

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

// handle_events

void GameLoop::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { _running = false; continue; }

        // El inventario tiene prioridad (modal)
        if (_inventory && _inventory->is_visible()) {
            if (_inventory->handle_event(event, _command_queue)) continue;
        }

        // Panel de stats (botón inventario)
        if (_stats && _stats->handle_event(event)) {
            if (_stats->inventory_button_clicked() && _inventory) {
                _inventory->toggle();
            }
            continue;
        }

        if (_chat && _chat->handle_event(event)) continue;

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            _running = false;
        } else if (event.type == SDL_WINDOWEVENT
                   && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            _camera.set_screen_size(event.window.data1, event.window.data2, StatsPanel::PANEL_W);
        } else if (event.type == SDL_MOUSEBUTTONDOWN
                   && event.button.button == SDL_BUTTON_LEFT) {
            handle_mouse_click(event.button.x, event.button.y);
        }
    }
}

// handle_input

void GameLoop::handle_input() {
    if (_inventory && _inventory->is_visible()) return;
    if (_chat && _chat->input_active()) return;
    if (!_command_queue && _player.is_moving()) return;

    Uint32 now = SDL_GetTicks();
    if (_command_queue && (now - _last_move_tick) < 200) return;

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

// update

void GameLoop::update(float dt) {
    if (_connected && !(*_connected)) {
        _running = false;
        return;
    }

    if (_map_queue) {
        MapaDTO map;
        if (_map_queue->try_pop(map))
            apply_map(std::move(map));
    }

    if (_snapshot_queue) {
        SnapshotDTO snap;
        while (_snapshot_queue->try_pop(snap))
            apply_snapshot(snap);
    }

    _player.update(dt);
    _camera.follow(_player);
}

void GameLoop::apply_map(const MapaDTO& map) {
    _map = map;
    _map_loaded = true;
}

void GameLoop::apply_snapshot(const SnapshotDTO& snap) {
    if (snap.entities)
        _last_entities = *snap.entities;
    _my_entity_id = snap.self_entity_id;
    _current_tick = snap.tick;

    if (_chat && snap.messages) {
        for (const auto& m : *snap.messages) _chat->add_message(m.text);
    }

    // Actualizar StatsPanel
    if (_stats) {
        uint8_t eq_weapon_item = 0;
        if (snap.equipped_wpn != 0xFF && snap.equipped_wpn < SnapshotDTO::INVENTORY_SIZE)
            eq_weapon_item = snap.inventory[snap.equipped_wpn];
        _stats->update(snap.hp, snap.max_hp,
                    snap.mp, snap.max_mp,
                    snap.gold, snap.level,
                    snap.meditating != 0,
                    snap.is_ghost   != 0,
                    snap.cls,
                    eq_weapon_item);
    }

    // Actualizar InventoryPanel
    if (_inventory) {
        _inventory->update(snap.inventory,
                           snap.equipped_wpn, snap.equipped_arm,
                           snap.equipped_helm, snap.equipped_shld);
    }

    // Guardar equipo e inventario del jugador propio
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
        _inv[i] = snap.inventory[i];
    _eq_wpn  = snap.equipped_wpn;
    _eq_arm  = snap.equipped_arm;
    _eq_helm = snap.equipped_helm;
    _eq_shld = snap.equipped_shld;

    if (!snap.entities) return;
    for (const auto& e : *snap.entities) {
        if (e.entity_id == snap.self_entity_id) {
            if (e.pos_x != static_cast<uint16_t>(_player.tile_x) ||
                e.pos_y != static_cast<uint16_t>(_player.tile_y)) {
                Direction dir = static_cast<Direction>(e.direction);
                _player.move_to(e.pos_x, e.pos_y, dir);
            }
            break;
        }
    }
}

// render

void GameLoop::render() {
    _renderer.SetDrawColor(0, 0, 0, 255);
    _renderer.Clear();
    render_floor();
    render_objects();
    render_entities();
    render_obj_sup();
    render_spells();

    int sw = _window.GetWidth();
    int sh = _window.GetHeight();

    if (_chat)      _chat->render(sw, sh);
    if (_stats)     _stats->render(sw, sh);
    if (_inventory) _inventory->render(sw, sh);

    _renderer.Present();
}

// ─── render_floor ────────────────────────────────────────────────────────────

void GameLoop::render_floor() {
    int screen_w = _window.GetWidth();
    int screen_h = _window.GetHeight();
    int map_w = _map_loaded ? _map.width  : MAP_SIZE;
    int map_h = _map_loaded ? _map.height : MAP_SIZE;

    int margin = 8;
    int first_x = std::max(0, -_camera.tile_to_screen_x(0) / TILE_SIZE - margin);
    int first_y = std::max(0, -_camera.tile_to_screen_y(0) / TILE_SIZE - margin);
    int last_x  = std::min(map_w - 1, first_x + screen_w / TILE_SIZE + margin * 2);
    int last_y  = std::min(map_h - 1, first_y + screen_h / TILE_SIZE + margin * 2);

    for (int ty = first_y; ty <= last_y; ty++) {
        for (int tx = first_x; tx <= last_x; tx++) {
            uint16_t floor_id = 0;
            if (_map_loaded)
                floor_id = _map.tiles[ty * _map.width + tx].floor_id;

            const TileEntry& entry = _tile_config.get(floor_id);
            if (entry.is_large()) continue;

            int sx = _camera.tile_to_screen_x(tx);
            int sy = _camera.tile_to_screen_y(ty);

            if (entry.path.empty()) {
                if (sx >= -TILE_SIZE && sx <= screen_w &&
                    sy >= -TILE_SIZE && sy <= screen_h) {
                    _renderer.SetDrawColor(0, 0, 0, 255);
                    _renderer.FillRect(SDL2pp::Rect(sx, sy, TILE_SIZE, TILE_SIZE));
                }
                continue;
            }

            SDL2pp::Rect dst(sx, sy, TILE_SIZE, TILE_SIZE);
            if (entry.has_src_rect()) {
                SDL2pp::Rect src(entry.src_x, entry.src_y, entry.src_w, entry.src_h);
                _renderer.Copy(_assets.get(entry.path), src, dst);
            } else {
                _renderer.Copy(_assets.get(entry.path), SDL2pp::NullOpt, dst);
            }
        }
    }

    for (int ty = 0; ty < map_h; ty++) {
        int sy_raw = _camera.tile_to_screen_y(ty);
        if (sy_raw > screen_h + 384) continue;
        if (sy_raw < -(384 + 384))   continue;

        for (int tx = 0; tx < map_w; tx++) {
            uint16_t floor_id = 0;
            if (_map_loaded)
                floor_id = _map.tiles[ty * _map.width + tx].floor_id;

            const TileEntry& entry = _tile_config.get(floor_id);
            if (!entry.is_large()) continue;

            int sx = _camera.tile_to_screen_x(tx);
            if (sx > screen_w + 384) continue;
            if (sx < -(384 + 384))   continue;

            int size_px = entry.tile_size * TILE_SIZE;
            SDL2pp::Rect dst(sx + entry.offset_x, sy_raw + entry.offset_y, size_px, size_px);
            _renderer.Copy(_assets.get(entry.path), SDL2pp::NullOpt, dst);
        }
    }
}

void GameLoop::render_obj_sup() {
    if (!_map_loaded) return;

    int screen_w = _window.GetWidth();
    int screen_h = _window.GetHeight();
    int map_w = _map.width;
    int map_h = _map.height;

    int margin_x = 8;
    int margin_y_up   = 6;
    int margin_y_down = 16;
    int first_x = std::max(0, -_camera.tile_to_screen_x(0) / TILE_SIZE - margin_x);
    int first_y = std::max(0, -_camera.tile_to_screen_y(0) / TILE_SIZE - margin_y_up);
    int last_x  = std::min(map_w - 1, first_x + screen_w / TILE_SIZE + margin_x * 2);
    int last_y  = std::min(map_h - 1, first_y + screen_h / TILE_SIZE + margin_y_up + margin_y_down);

    for (int ty = first_y; ty <= last_y; ty++) {
        for (int tx = first_x; tx <= last_x; tx++) {
            uint16_t obj_id = _map.tiles[ty * map_w + tx].object_superior_id;
            if (obj_id == 0) continue;

            const ObjectSupEntry& entry = _obj_sup_config.get(obj_id);
            if (entry.frames.empty()) continue;

            int frame_idx = (_current_tick / OBJ_SUP_TICKS_PER_FRAME)
                            % static_cast<int>(entry.frames.size());

            int obj_h = entry.size_tiles  * TILE_SIZE;
            int obj_w = entry.width_tiles * TILE_SIZE;

            int sx = _camera.tile_to_screen_x(tx);
            int sy = _camera.tile_to_screen_y(ty);

            SDL2pp::Rect dst(
                sx - (obj_w - TILE_SIZE) / 2 + entry.offset_x,
                sy - obj_h + TILE_SIZE + entry.offset_y,
                obj_w,
                obj_h
            );

            _renderer.Copy(_assets.get(entry.frames[frame_idx]),
                           SDL2pp::NullOpt, dst);
        }
    }
}

void GameLoop::render_objects() {
    if (!_map_loaded) return;
    // TODO: renderizar object_id con AssetManager
}

void GameLoop::render_entities() {
    std::sort(_last_entities.begin(), _last_entities.end(),
        [](const EntityDTO& a, const EntityDTO& b) {
            return a.pos_y < b.pos_y;
        });

    // Mapeo ItemId → path de armadura (body overlay)
    static const std::unordered_map<uint8_t, std::string> armor_paths = {
        { 10, "assets/sprites/equipment/armor/guerrero_ejecutor.png" },
        { 11, "assets/sprites/equipment/armor/paladin_real.png"      },
    };

    // Mapeo ItemId → path de arma
    static const std::unordered_map<uint8_t, std::string> weapon_paths = {
        {  1, "assets/sprites/weapons/sword/espada_comun.png"         },
        {  2, "assets/sprites/weapons/axe/hacha_hierro.png"           },
        {  3, "assets/sprites/weapons/hammer/martillo_comun.png"      },
        {  4, "assets/sprites/weapons/bow/arco_simple_madera.png"     },
        {  5, "assets/sprites/weapons/bow/arco_compuesto_oro.png"     },
        {  6, "assets/sprites/weapons/flute/flauta_elfica.png"        },
        {  7, "assets/sprites/weapons/staff/baculo_esmeralda.png"     },
        {  8, "assets/sprites/weapons/stick/vara_fresno.png"          },
    };

    for (const auto& e : _last_entities) {
        int screen_x = _camera.world_to_screen_x(
            static_cast<float>(e.pos_x * TILE_SIZE));
        int screen_y = _camera.world_to_screen_y(
            static_cast<float>(e.pos_y * TILE_SIZE));

        // ── Items en el suelo ──
        if (e.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR)) {
            // Variantes visuales por item_id (e.direction = sprite_variant)
            static const std::unordered_map<uint8_t, std::vector<std::string>> item_variants = {
                {  1, { "assets/sprites/weapons/sword/espada_comun.png",
                        "assets/sprites/weapons/sword/espada_oscura.png" }},
                {  2, { "assets/sprites/weapons/axe/hacha_hierro.png",
                        "assets/sprites/weapons/axe/hacha_epica.png" }},
                {  3, { "assets/sprites/weapons/hammer/martillo_comun.png",
                        "assets/sprites/weapons/hammer/martillo_epico.png",
                        "assets/sprites/weapons/hammer/martillo_legendario.png" }},
                {  4, { "assets/sprites/weapons/bow/arco_simple_madera.png",
                        "assets/sprites/weapons/bow/arco_simple_amatista.png" }},
                {  5, { "assets/sprites/weapons/bow/arco_compuesto_oro.png",
                        "assets/sprites/weapons/bow/arco_compuesto_infernal.png" }},
                {  6, { "assets/sprites/weapons/flute/flauta_elfica.png" }},
                {  7, { "assets/sprites/weapons/staff/baculo_esmeralda.png",
                        "assets/sprites/weapons/staff/baculo_egipcio.png",
                        "assets/sprites/weapons/staff/baculo_esqueletico.png" }},
                {  8, { "assets/sprites/weapons/stick/vara_fresno.png",
                        "assets/sprites/weapons/stick/vara_cuarzo.png",
                        "assets/sprites/weapons/stick/vara_muerdago.png" }},
                { 10, { "assets/sprites/equipment/armor/clerigo_blanco.png",
                        "assets/sprites/equipment/armor/clerigo_negro.png",
                        "assets/sprites/equipment/armor/mago_comun.png",
                        "assets/sprites/equipment/armor/mago_real.png" }},
                { 11, { "assets/sprites/equipment/armor/guerrero_ejecutor.png",
                        "assets/sprites/equipment/armor/guerrero_epico.png",
                        "assets/sprites/equipment/armor/paladin_magico.png",
                        "assets/sprites/equipment/armor/paladin_real.png" }},
                { 30, { "assets/sprites/equipment/shield/escudo_tortuga.png" }},
                { 31, { "assets/sprites/equipment/shield/escudo_hierro.png",
                        "assets/sprites/equipment/shield/escudo_boca.png" }},
                // Items sin variante
                { static_cast<uint8_t>(ItemId::BLOOD_STAIN),
                        { "assets/sprites/stage/sangre.png" }},
                { static_cast<uint8_t>(ItemId::GOLD_PILE),
                        { "assets/sprites/items/oro.png" }},
                { static_cast<uint8_t>(ItemId::HEALTH_POTION),
                        { "assets/sprites/items/pocion_vida.png" }},
                { static_cast<uint8_t>(ItemId::MANA_POTION),
                        { "assets/sprites/items/pocion_mana.png" }},
            };

            std::string path = "assets/sprites/items/oro.png"; // fallback
            auto vit = item_variants.find(e.sprite_id);
            if (vit != item_variants.end() && !vit->second.empty()) {
                uint8_t variant = e.direction % static_cast<uint8_t>(vit->second.size());
                path = vit->second[variant];
            }

            SDL2pp::Rect dst(screen_x, screen_y, TILE_SIZE, TILE_SIZE);
            SDL2pp::Texture& tex = _assets.get(path);
            if (tex.GetWidth() == 256 && tex.GetHeight() == 256)
                _renderer.Copy(tex, SDL2pp::Rect(0, 192, 48, 64), dst);
            else
                _renderer.Copy(tex, SDL2pp::NullOpt, dst);
            continue;
        }

        // ── Jugadores y NPCs ──
        Direction dir = static_cast<Direction>(e.direction);
        bool moving = false;

        if (e.entity_id == _my_entity_id) {
            screen_x = _camera.world_to_screen_x(_player.pixel_x());
            screen_y = _camera.world_to_screen_y(_player.pixel_y());
            moving = _player.is_moving();
        }

        // ── NPC: spritesheet propio con apariencia random por entidad ──
        if (e.entity_type == static_cast<uint8_t>(EntityType::NPC)) {
            // {path, sheet_w, sheet_h, cols, rows}
            struct NpcSheet { std::string path; int w, h, cols, rows; };
            static const std::unordered_map<uint8_t, std::vector<NpcSheet>> npc_sheets = {
                { 1, { // GOBLIN
                    { "assets/npcs/goblin/duende_ladron.png",      256,  256, 6, 4 },
                    { "assets/npcs/goblin/duende_zombie_real.png",  512,  512, 5, 4 },
                }},
                { 2, { // SKELETON
                    { "assets/npcs/skeleton/esqueleto_magico.png", 256, 256, 6, 4 },
                    { "assets/npcs/skeleton/esqueleto_muerte.png", 256, 256, 6, 4 },
                    { "assets/npcs/skeleton/esqueleto_oscuro.png", 256, 256, 6, 4 },
                }},
                { 3, { // ZOMBIE
                    { "assets/npcs/zombie/zombie_comun.png",   256,  256, 4, 4 },
                    { "assets/npcs/zombie/zombie_magico.png",  256,  256, 4, 4 },
                    { "assets/npcs/zombie/zombie_poseido.png", 512,  512, 5, 4 },
                }},
                { 4, { // SPIDER
                    { "assets/npcs/spider/araña_comun.png",      512,  512, 7, 4 },
                    { "assets/npcs/spider/araña_endurecida.png", 1024, 1024, 5, 4 },
                }},
                { 5, { // ORC
                    { "assets/npcs/orc/orco_errante.png", 512, 512, 6, 4 },
                    { "assets/npcs/orc/orco_fuego.png",   512, 512, 5, 4 },
                }},
                { 6, { // GOLEM
                    { "assets/npcs/golem/golem_moribundo.png", 296,  202, 4, 4 },
                    { "assets/npcs/golem/golem_reforzado.png", 512,  512, 6, 4 },
                    { "assets/npcs/golem/golem_tierra.png",   1024, 1024, 6, 4 },
                }},
            };

            auto sit = npc_sheets.find(e.sprite_id);
            if (sit != npc_sheets.end() && !sit->second.empty()) {
                const auto& sheets = sit->second;
                const NpcSheet& s = sheets[e.entity_id % sheets.size()];
                _anim.render_npc(_renderer, _assets,
                                 s.path, s.w, s.h, s.cols, s.rows,
                                 dir, screen_x, screen_y,
                                 _current_tick, moving);
            }
            continue;
        }

        // Construir equipo visual solo para el jugador propio
        EquipVisual equip{};
        const EquipVisual* equip_ptr = nullptr;

        if (e.entity_id == _my_entity_id) {
            // Arma
            if (_eq_wpn != 0xFF && _eq_wpn < SnapshotDTO::INVENTORY_SIZE) {
                auto wit = weapon_paths.find(_inv[_eq_wpn]);
                if (wit != weapon_paths.end())
                    equip.weapon_path = wit->second;
            }
            // Armadura
            if (_eq_arm != 0xFF && _eq_arm < SnapshotDTO::INVENTORY_SIZE) {
                auto ait = armor_paths.find(_inv[_eq_arm]);
                if (ait != armor_paths.end())
                    equip.armor_path = ait->second;
            }
            // Casco: se agrega cuando se seleccionen los src_rect
            equip_ptr = &equip;
        }

        const SpriteEntry& sprite = _sprite_config.get(e.sprite_id);
        _anim.render(_renderer, _assets,
                     sprite.body_path, sprite.head_path,
                     e.sprite_id, dir,
                     screen_x, screen_y,
                     _current_tick, moving,
                     equip_ptr);
    }
}

void GameLoop::handle_mouse_click(int mouse_x, int mouse_y) {
    if (!_command_queue) return;

    // El panel de stats ocupa el borde derecho; no procesar clicks ahí
    if (mouse_x >= _window.GetWidth() - StatsPanel::PANEL_W) return;

    int world_x = mouse_x - _camera.tile_to_screen_x(0);
    int world_y = mouse_y - _camera.tile_to_screen_y(0);
    int tile_x = world_x / TILE_SIZE;
    int tile_y = world_y / TILE_SIZE;

    for (const auto& e : _last_entities) {
        if (e.entity_id == _my_entity_id) continue;
        if (e.pos_x == tile_x && e.pos_y == tile_y) {
            if (_stats && _stats->cast_mode_active() && _stats->selected_spell() != 0) {
                uint8_t spell = _stats->selected_spell();
                _command_queue->push(Command::cast_spell(e.entity_id, spell));
                // Efecto visual inmediato en posición del caster
                spawn_spell_effect(spell,
                    static_cast<uint16_t>(_player.tile_x),
                    static_cast<uint16_t>(_player.tile_y));
                _chat->add_message("Lanzando hechizo a " + (e.username.empty()
                    ? std::string("#") + std::to_string(e.entity_id)
                    : e.username));
            } else {
                _command_queue->push(Command::attack(e.entity_id));
                _chat->add_message("Atacando a " + (e.username.empty()
                    ? std::string("#") + std::to_string(e.entity_id)
                    : e.username));
            }
            return;
        }
    }
}