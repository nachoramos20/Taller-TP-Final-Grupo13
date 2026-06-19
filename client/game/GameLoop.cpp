#include "GameLoop.h"
#include <algorithm>
#include <array>
#include <cmath>
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
      _map_queue(nullptr), _connected(nullptr), _audio(nullptr),
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
    _pos_label = std::make_unique<PositionLabel>(renderer, CHAT_FONT_PATH);
    _small_font = TTF_OpenFont(CHAT_FONT_PATH, 10);
    load_item_textures();
}

GameLoop::GameLoop(SDL2pp::Window& window, SDL2pp::Renderer& renderer,
                   Queue<Command>* command_queue,
                   Queue<SnapshotDTO>* snapshot_queue,
                   Queue<MapaDTO>* map_queue,
                   std::atomic<bool>* connected,
                   AudioManager* audio)
    : _window(window), _renderer(renderer),
      _camera(window.GetWidth(), window.GetHeight(), StatsPanel::PANEL_W),
      _running(false),
      _command_queue(command_queue),
      _snapshot_queue(snapshot_queue),
      _map_queue(map_queue),
      _connected(connected),
      _audio(audio),
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
    _pos_label = std::make_unique<PositionLabel>(renderer, CHAT_FONT_PATH);
    _small_font = TTF_OpenFont(CHAT_FONT_PATH, 10);
    _chat->on_submit([this](const std::string& text) {
        if (!_command_queue) return;
        _command_queue->push(Command::chat(text));
        _chat->add_message("> " + text);

        if (_audio && _shop_npc_id != -1) {
            float dist = 0.0f;
            for (const auto& e : _last_entities)
                if (e.entity_id == _shop_npc_id) { dist = dist_to_player_tiles(e.pos_x, e.pos_y); break; }

            if (text.rfind("/listar", 0) == 0) {
                static const std::vector<std::string> busco = {
                    "assets/sounds/effects/npcs/comerciante/lo_pides_lo_tienes.wav",
                };
                _audio->play_random_effect_at(busco, dist);
            } else if (text.rfind("/comprar", 0) == 0) {
                static const std::vector<std::string> elijo = {
                    "assets/sounds/effects/npcs/comerciante/una_gran_eleccion.wav",
                };
                _audio->play_random_effect_at(elijo, dist);
            }
        }

        if (_audio && _bank_npc_id != -1) {
            float dist = 0.0f;
            for (const auto& e : _last_entities)
                if (e.entity_id == _bank_npc_id) { dist = dist_to_player_tiles(e.pos_x, e.pos_y); break; }

            if (text.rfind("/depositar", 0) == 0) {
                static const std::vector<std::string> depositar = {
                    "assets/sounds/effects/npcs/banquero/cuidamos_sus_cosas_mejor_que_usted.wav",
                };
                _audio->play_random_effect_at(depositar, dist);
            } else if (text.rfind("/retirar", 0) == 0) {
                static const std::vector<std::string> retirar = {
                    "assets/sounds/effects/npcs/banquero/puede_confiar_en_nosotros.wav",
                };
                _audio->play_random_effect_at(retirar, dist);
            }
        }

        if (_audio && _priest_npc_id != -1) {
            float dist = 0.0f;
            for (const auto& e : _last_entities)
                if (e.entity_id == _priest_npc_id) { dist = dist_to_player_tiles(e.pos_x, e.pos_y); break; }

            if (text.rfind("/curar", 0) == 0) {
                static const std::vector<std::string> curar = {
                    "assets/sounds/effects/npcs/sacerdote/orare_por_ti.wav",
                    "assets/sounds/effects/npcs/sacerdote/curar.wav",
                };
                _audio->queue_speech_sequence(curar, dist);
            } else if (text.rfind("/resucitar", 0) == 0) {
                static const std::vector<std::string> resucitar = {
                    "assets/sounds/effects/npcs/sacerdote/resucitar_con_sacerdote.wav",
                };
                _audio->play_random_effect_at(resucitar, dist);
            }
        }
    });

    load_item_textures();
}

void GameLoop::spawn_spell_effect(uint8_t spell_id, uint16_t pos_x, uint16_t pos_y) {
    struct SpellInfo {
        const char* path;
        int sheet_cols;
        int frame_w;
        int frame_h;
        std::vector<int> frame_indices;
    };
    static const std::unordered_map<uint8_t, SpellInfo> spell_info = {
        { 1, { "assets/sprites/spells/explosion.png",            5, 192, 192,
               { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 } }},
        { 2, { "assets/sprites/spells/area_veneno.png",          4, 128, 128,
               { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 } }},
        { 3, { "assets/sprites/spells/explosion_calaverica.png", 3, 145, 145,
               { 0, 1, 2, 3, 4, 5, 6 } }},
        { 4, { "assets/sprites/spells/orbe_hielo.png",           8, 128, 128,
               { 0, 1, 2, 3, 4, 8, 9, 10, 11, 12, 16, 17, 18, 19, 20,
                 24, 25, 26, 27, 28, 32, 33, 34, 35, 36, 40, 41, 42, 43, 44 } }},
        { 5, { "assets/sprites/spells/tornado_gravitatorio.png", 5, 120, 255,
               { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } }},
        { 6, { "assets/sprites/spells/tormenta_electrica.png",   4, 128, 128,
               { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 } }},
        { 7, { "assets/sprites/spells/orbe_vacio.png",           6, 112, 112,
               { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } }},
        { 8, { "assets/sprites/spells/brecha_vacio.png",         7, 146, 146,
               { 0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18,
                 21, 22, 23, 24, 25 } }},
        { 9, { "assets/sprites/spells/tornado_oscuridad.png",    5, 120, 255,
               { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } }},
    };
    auto it = spell_info.find(spell_id);
    if (it == spell_info.end()) return;

    SpellEffect fx{};
    fx.spell_id   = spell_id;
    fx.pos_x      = pos_x;
    fx.pos_y      = pos_y;
    fx.start_tick = _current_tick;
    fx.sheet_cols = it->second.sheet_cols;
    fx.frame_w    = it->second.frame_w;
    fx.frame_h    = it->second.frame_h;
    fx.frame_indices = it->second.frame_indices;
    fx.path       = it->second.path;
    _spell_effects.push_back(fx);
}

void GameLoop::render_spells() {
    _spell_effects.erase(
        std::remove_if(_spell_effects.begin(), _spell_effects.end(),
            [&](const SpellEffect& fx) {
                int total = static_cast<int>(fx.frame_indices.size());
                uint32_t elapsed = _current_tick - fx.start_tick;
                return static_cast<int>(elapsed / SPELL_TICKS_PER_FRAME) >= total;
            }),
        _spell_effects.end());

    struct SpellRenderInfo {
        int display_w;
        int display_h;
        int offset_x;
        int offset_y;
    };
    static const std::unordered_map<uint8_t, SpellRenderInfo> spell_render = {
        { 1, { 150, 150, -75, -124 } },
        { 2, {  96,  96, -48,  -80 } },
        { 3, { 120, 120, -60, -104 } },
        { 4, {  96,  96, -48,  -80 } },
        { 5, {  80, 170, -40, -144 } },
        { 6, {  96,  96, -48,  -80 } },
        { 7, {  96,  96, -48,  -80 } },
        { 8, { 120, 120, -60, -104 } },
        { 9, {  80, 170, -40, -144 } },
    };

    for (const auto& fx : _spell_effects) {
        int total = static_cast<int>(fx.frame_indices.size());
        if (total == 0 || fx.sheet_cols <= 0 || fx.frame_w <= 0 || fx.frame_h <= 0) continue;

        uint32_t elapsed = _current_tick - fx.start_tick;
        int frame = static_cast<int>(elapsed / SPELL_TICKS_PER_FRAME) % total;
        int sheet_frame = fx.frame_indices[frame];

        int col = sheet_frame % fx.sheet_cols;
        int row = sheet_frame / fx.sheet_cols;

        SDL2pp::Texture& tex = _assets.get(fx.path);
        SDL2pp::Rect src(col * fx.frame_w, row * fx.frame_h, fx.frame_w, fx.frame_h);

        // Calcular posición en pantalla centrada sobre el tile del objetivo
        int center_x = _camera.world_to_screen_x(static_cast<float>(fx.pos_x * TILE_SIZE))
                       + TILE_SIZE / 2;
        int center_y = _camera.world_to_screen_y(static_cast<float>(fx.pos_y * TILE_SIZE))
                       + TILE_SIZE / 2;

        // Obtener dimensiones hardcodeadas para este hechizo
        auto rit = spell_render.find(fx.spell_id);
        int dw, dh, ox, oy;
        if (rit != spell_render.end()) {
            dw = rit->second.display_w;
            dh = rit->second.display_h;
            ox = rit->second.offset_x;
            oy = rit->second.offset_y;
        } else {
            // fallback genérico
            dw = TILE_SIZE * 2;
            dh = TILE_SIZE * 2;
            ox = -TILE_SIZE / 2;
            oy = -TILE_SIZE;
        }

        SDL2pp::Rect dst(center_x + ox, center_y + oy, dw, dh);
        _renderer.Copy(tex, src, dst);
    }
}

void GameLoop::render_deaths() {
    uint32_t now = SDL_GetTicks();

    _death_effects.erase(
        std::remove_if(_death_effects.begin(), _death_effects.end(),
            [&](const DeathEffect& d) {
                return (now - d.start_ms) >= DEATH_DURATION_MS;
            }),
        _death_effects.end());

    for (const auto& d : _death_effects) {
        uint32_t elapsed_ms = now - d.start_ms;
        int frame = std::clamp(
            static_cast<int>(elapsed_ms / DEATH_FRAME_MS),
            0, DEATH_FRAMES - 1);

        std::string path = "assets/sprites/stage/sangre_"
                         + std::to_string(frame + 1) + ".png";
        int sx = _camera.tile_to_screen_x(static_cast<int>(d.pos_x));
        int sy = _camera.tile_to_screen_y(static_cast<int>(d.pos_y));
        SDL2pp::Rect dst(sx, sy, TILE_SIZE, TILE_SIZE);
        _renderer.Copy(_assets.get(path), SDL2pp::NullOpt, dst);
    }
}

void GameLoop::spawn_projectile(uint16_t from_x, uint16_t from_y,
                                uint16_t to_x, uint16_t to_y, bool is_magic) {
    Projectile p{};
    p.from_x     = from_x;
    p.from_y     = from_y;
    p.to_x       = to_x;
    p.to_y       = to_y;
    p.start_tick = _current_tick;
    p.is_magic   = is_magic;
    _projectiles.push_back(p);
}

float GameLoop::dist_to_player_tiles(uint16_t x, uint16_t y) const {
    float dx = static_cast<float>(x) - static_cast<float>(_player.tile_x);
    float dy = static_cast<float>(y) - static_cast<float>(_player.tile_y);
    return std::sqrt(dx * dx + dy * dy);
}

void GameLoop::play_attack_sound(uint8_t weapon_item, uint16_t x, uint16_t y) {
    if (!_audio) return;
    static const std::vector<std::string> espada = {"assets/sounds/effects/combat/espadazo.wav"};
    static const std::vector<std::string> hacha   = {"assets/sounds/effects/combat/hachazo_y_golpe_de_clavado.wav"};
    static const std::vector<std::string> martillo = {"assets/sounds/effects/combat/martillazo.wav"};
    static const std::vector<std::string> generico = {
        "assets/sounds/effects/combat/golpe_con_arma.wav",
        "assets/sounds/effects/combat/golpe_con_arma_2.wav",
        "assets/sounds/effects/combat/apunalada.wav",
    };
    static const std::vector<std::string> flecha = {"assets/sounds/effects/combat/flecha.wav"};
    static const std::vector<std::string> flecha_magica = {"assets/sounds/effects/combat/flecha_magica.wav"};
    static const std::vector<std::string> curar = {
        "assets/sounds/effects/magic/sonido_como_de_curacion_divina.wav",
        "assets/sounds/effects/magic/sonido_como_de_curacion_divina_2.wav",
    };
    static const std::vector<std::string> misil = {
        "assets/sounds/effects/magic/sonido_de_onda.wav",
        "assets/sounds/effects/magic/sonido_de_onda_2.wav",
    };
    static const std::vector<std::string> explosion = {"assets/sounds/effects/combat/explosion.wav"};

    float dist = dist_to_player_tiles(x, y);
    ItemId item = static_cast<ItemId>(weapon_item);
    if (weapon_item == 0) {
        _audio->play_random_effect_at(generico, dist);
    } else if (item == ItemId::SIMPLE_BOW) {
        // Arco simple: flecha común.
        _audio->play_random_effect_at(flecha, dist);
    } else if (item == ItemId::COMPOUND_BOW) {
        // Arco compuesto, más poderoso: flecha mágica.
        _audio->play_random_effect_at(flecha_magica, dist);
    } else if (item == ItemId::ASH_STICK) {
        // Vara de fresno: hechizo "flecha mágica".
        _audio->play_random_effect_at(flecha_magica, dist);
    } else if (item == ItemId::ELVEN_FLUTE) {
        // Flauta élfica: hechizo "curar".
        _audio->play_random_effect_at(curar, dist);
    } else if (item == ItemId::NUDOSO_STAFF) {
        // Báculo nudoso: hechizo "misil".
        _audio->play_random_effect_at(misil, dist);
    } else if (item == ItemId::GEMMED_STAFF) {
        // Báculo engarzado: hechizo "explosion".
        _audio->play_random_effect_at(explosion, dist);
    } else if (item == ItemId::SWORD) {
        _audio->play_random_effect_at(espada, dist);
    } else if (item == ItemId::AXE) {
        _audio->play_random_effect_at(hacha, dist);
    } else if (item == ItemId::HAMMER) {
        _audio->play_random_effect_at(martillo, dist);
    } else {
        _audio->play_random_effect_at(generico, dist);
    }
}

void GameLoop::play_spell_sound(uint8_t spell_id, uint16_t x, uint16_t y) {
    if (!_audio || spell_id == 0) return;
    static const std::vector<std::string> hechizo = {
        "assets/sounds/effects/combat/explosion.wav",
        "assets/sounds/effects/magic/sonido_similar_al_de_lanzar_un_hechizo.wav",
        "assets/sounds/effects/magic/sonido_similar_al_de_lanzar_un_hechizo_2.wav",
        "assets/sounds/effects/magic/sonido_similar_al_de_lanzar_un_hechizo_3.wav",
        "assets/sounds/effects/magic/apertura_magica.wav",
        "assets/sounds/effects/magic/sonido_de_onda.wav",
        "assets/sounds/effects/magic/sonido_de_onda_2.wav",
        "assets/sounds/effects/magic/resorte_explosivo.wav",
        "assets/sounds/effects/magic/sonido_como_de_portal_magico.wav",
        "assets/sounds/effects/magic/sonido_como_de_portal_magico_2.wav",
        "assets/sounds/effects/magic/sonido_de_runa.wav",
    };
    _audio->play_random_effect_at(hechizo, dist_to_player_tiles(x, y));
}

void GameLoop::play_npc_death_sound(uint8_t npc_sprite_id, uint16_t x, uint16_t y) {
    if (!_audio) return;
    static const std::vector<std::string> bestia = {
        "assets/sounds/effects/creatures/sonido_de_bestia.wav",
    };
    static const std::vector<std::string> orco = {
        "assets/sounds/effects/creatures/bandido.wav",
        "assets/sounds/effects/creatures/bandido_2.wav",
    };
    static const std::vector<std::string> zombie = {"assets/sounds/effects/creatures/zombie.wav"};
    static const std::vector<std::string> esqueleto = {"assets/sounds/effects/creatures/sonido_no_muerto.wav"};

    static const std::vector<std::string> golpe_final = {
        "assets/sounds/effects/combat/espadazo_y_sangre_cayendo.wav",
    };

    float dist = dist_to_player_tiles(x, y);
    switch (static_cast<NpcId>(npc_sprite_id)) {
        case NpcId::ORC:      _audio->play_random_effect_at(orco, dist);     break;
        case NpcId::ZOMBIE:   _audio->play_random_effect_at(zombie, dist);   break;
        case NpcId::SKELETON: _audio->play_random_effect_at(esqueleto, dist);break;
        case NpcId::GOBLIN:
        case NpcId::SPIDER:
        case NpcId::GOLEM:    _audio->play_random_effect_at(bestia, dist);   break;
        default: break;
    }

    // Golpe final con sangre: solo si tengo la espada equipada.
    uint8_t my_weapon = (_eq_wpn != 0xFF && _eq_wpn < SnapshotDTO::INVENTORY_SIZE)
                         ? _inv[_eq_wpn] : 0;
    ItemId item = static_cast<ItemId>(my_weapon);
    if (item == ItemId::SWORD)
        _audio->play_random_effect_at(golpe_final, dist);
}

void GameLoop::play_player_death_sound(uint16_t x, uint16_t y) {
    if (!_audio) return;
    static const std::vector<std::string> muerte = {
        "assets/sounds/effects/combat/sonido_de_muerte_hombre.wav",
        "assets/sounds/effects/combat/grito_de_hombre.wav",
    };
    _audio->play_random_effect_at(muerte, dist_to_player_tiles(x, y));
}

void GameLoop::render_projectiles() {
    _projectiles.erase(
        std::remove_if(_projectiles.begin(), _projectiles.end(),
            [&](const Projectile& p) {
                return (_current_tick - p.start_tick) >= PROJECTILE_DURATION_TICKS;
            }),
        _projectiles.end());

    for (const auto& p : _projectiles) {
        float t = static_cast<float>(_current_tick - p.start_tick)
                / static_cast<float>(PROJECTILE_DURATION_TICKS);
        t = std::clamp(t, 0.0f, 1.0f);

        float world_x = (p.from_x + (p.to_x - p.from_x) * t) * TILE_SIZE + TILE_SIZE / 2.0f;
        float world_y = (p.from_y + (p.to_y - p.from_y) * t) * TILE_SIZE + TILE_SIZE / 2.0f;

        int sx = _camera.world_to_screen_x(world_x);
        int sy = _camera.world_to_screen_y(world_y);

        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_Color color = p.is_magic ? SDL_Color{120, 180, 255, 255}
                                      : SDL_Color{230, 210, 120, 255};
        SDL_SetRenderDrawColor(_renderer.Get(), color.r, color.g, color.b, color.a);
        SDL_Rect dot{ sx - 3, sy - 3, 6, 6 };
        SDL_RenderFillRect(_renderer.Get(), &dot);
    }
}

void GameLoop::load_item_textures() {
    if (!_inventory) return;

    // Mapeo ItemId → path del sprite basado en el catálogo del servidor
    static const std::vector<std::pair<uint8_t, std::string>> item_paths = {
        // Armas cuerpo a cuerpo
        { static_cast<uint8_t>(ItemId::SWORD),              "assets/sprites/weapons/sword/espada.png" },
        { static_cast<uint8_t>(ItemId::AXE),                "assets/sprites/weapons/axe/hacha.png" },
        { static_cast<uint8_t>(ItemId::HAMMER),             "assets/sprites/weapons/hammer/martillo.png" },

        // Armas a distancia
        { static_cast<uint8_t>(ItemId::SIMPLE_BOW),         "assets/sprites/weapons/bow/arco_simple.png" },
        { static_cast<uint8_t>(ItemId::COMPOUND_BOW),       "assets/sprites/weapons/bow/arco_compuesto.png" },

        // Armas mágicas
        { static_cast<uint8_t>(ItemId::ELVEN_FLUTE),        "assets/sprites/weapons/flute/flauta_elfica.png" },
        { static_cast<uint8_t>(ItemId::ASH_STICK),          "assets/sprites/weapons/stick/vara_fresno.png" },
        { static_cast<uint8_t>(ItemId::NUDOSO_STAFF),       "assets/sprites/weapons/staff/baculo_nudoso.png" },
        { static_cast<uint8_t>(ItemId::GEMMED_STAFF),       "assets/sprites/weapons/staff/baculo_engarzado.png" },

        // Armaduras
        { static_cast<uint8_t>(ItemId::LEATHER_ARMOR),      "assets/sprites/equipment/armor/clerigo_blanco.png" },
        { static_cast<uint8_t>(ItemId::CLERIC_BLACK_ARMOR), "assets/sprites/equipment/armor/clerigo_negro.png" },
        { static_cast<uint8_t>(ItemId::MAGE_COMMON_ARMOR),  "assets/sprites/equipment/armor/mago_comun.png" },
        { static_cast<uint8_t>(ItemId::MAGE_ROYAL_ARMOR),   "assets/sprites/equipment/armor/mago_real.png" },
        { static_cast<uint8_t>(ItemId::PLATE_ARMOR),        "assets/sprites/equipment/armor/guerrero_ejecutor.png" },
        { static_cast<uint8_t>(ItemId::WARRIOR_EPIC_ARMOR), "assets/sprites/equipment/armor/guerrero_epico.png" },
        { static_cast<uint8_t>(ItemId::PALADIN_MAGIC_ARMOR),"assets/sprites/equipment/armor/paladin_magico.png" },
        { static_cast<uint8_t>(ItemId::PALADIN_ROYAL_ARMOR),"assets/sprites/equipment/armor/paladin_real.png" },
        
        // Cascos
        { static_cast<uint8_t>(ItemId::HOOD),               "assets/sprites/equipment/helmet/capucha_tirada.png" },
        { static_cast<uint8_t>(ItemId::IRON_HELMET),        "assets/sprites/equipment/helmet/casco_de_hierro_tirado.png" },
        { static_cast<uint8_t>(ItemId::MAGIC_HAT),          "assets/sprites/equipment/helmet/sombrero_mago_tirado.png" },
        
        // Escudos
        { static_cast<uint8_t>(ItemId::TURTLE_SHIELD),      "assets/sprites/equipment/shield/escudo_tortuga.png" },
        { static_cast<uint8_t>(ItemId::IRON_SHIELD),        "assets/sprites/equipment/shield/escudo_hierro.png" },
        { static_cast<uint8_t>(ItemId::BOCA_SHIELD),        "assets/sprites/equipment/shield/escudo_boca.png" },
        
        // Pociones
        { static_cast<uint8_t>(ItemId::HEALTH_POTION),      "assets/sprites/items/pocion_vida.png" },
        { static_cast<uint8_t>(ItemId::MANA_POTION),        "assets/sprites/items/pocion_mana.png" },
        
        // Oro
        { static_cast<uint8_t>(ItemId::GOLD_PILE),          "assets/sprites/items/oro.png" },
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

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x &&
            !(_inventory && _inventory->is_visible()) &&
            !(_chat && _chat->input_active())) {
            _pos_label->toggle_visibility();
            continue;
        }

        // Tecla dedicada para abrir/cerrar el inventario (no pasa por chat)
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_i &&
            !(_chat && _chat->input_active())) {
            if (_inventory) _inventory->toggle();
            continue;
        }

        // Tecla dedicada para tirar el item seleccionado del inventario
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q &&
            _inventory && _inventory->is_visible() &&
            !(_chat && _chat->input_active()) && _command_queue) {
            int slot = _inventory->selected_slot();
            if (slot >= 0)
                _command_queue->push(Command::drop(static_cast<uint8_t>(slot)));
            continue;
        }

        // Teclas dedicadas para meditar / resucitar / tomar (sin pasar por chat)
        if (event.type == SDL_KEYDOWN &&
            !(_inventory && _inventory->is_visible()) &&
            !(_chat && _chat->input_active())) {
            if (event.key.keysym.sym == SDLK_m && _command_queue) {
                _command_queue->push(Command::meditate());
                continue;
            }
            if (event.key.keysym.sym == SDLK_r && _command_queue) {
                _command_queue->push(Command::resurrect());
                continue;
            }
            if (event.key.keysym.sym == SDLK_e && _command_queue) {
                _command_queue->push(Command::pick_item());
                continue;
            }
        }

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
    _pos_label->update(_player.tile_x, _player.tile_y);
    if (_audio) _audio->update();
}

void GameLoop::apply_map(const MapaDTO& map) {
    _map = map;
    _map_loaded = true;
}

void GameLoop::apply_snapshot(const SnapshotDTO& snap) {
    _my_entity_id = snap.self_entity_id;
    _current_tick = snap.tick;

    if (snap.entities) {
        // Detectar NPCs que murieron (estaban en el snapshot anterior, ya no están)
        for (const auto& prev : _last_entities) {
            if (prev.entity_type != static_cast<uint8_t>(EntityType::NPC)) continue;
            bool found = false;
            for (const auto& ne : *snap.entities)
                if (ne.entity_id == prev.entity_id) { found = true; break; }
            if (!found) {
                _death_effects.push_back({prev.pos_x, prev.pos_y, SDL_GetTicks()});
                play_npc_death_sound(prev.sprite_id, prev.pos_x, prev.pos_y);
            }
        }

        // Detectar otros jugadores que acaban de morir (pasan a fantasma).
        // La propia muerte se maneja aparte con _was_ghost.
        for (const auto& ne : *snap.entities) {
            if (ne.entity_type != static_cast<uint8_t>(EntityType::PLAYER)) continue;
            if (ne.entity_id == _my_entity_id || ne.is_ghost == 0) continue;
            for (const auto& prev : _last_entities) {
                if (prev.entity_id == ne.entity_id && prev.is_ghost == 0) {
                    play_player_death_sound(ne.pos_x, ne.pos_y);
                    break;
                }
            }
        }

        _last_entities = *snap.entities;
    }

    // Si me alejé del comerciante con el que estaba hablando, despedida.
    if (_shop_npc_id != -1) {
        static constexpr float SHOP_RANGE_TILES = 2.0f;
        const EntityDTO* shop_npc = nullptr;
        for (const auto& e : _last_entities)
            if (e.entity_id == _shop_npc_id) { shop_npc = &e; break; }

        bool left = (shop_npc == nullptr)
                  || (dist_to_player_tiles(shop_npc->pos_x, shop_npc->pos_y) > SHOP_RANGE_TILES);
        if (left) {
            if (_audio) {
                static const std::vector<std::string> vuelve_pronto = {
                    "assets/sounds/effects/npcs/comerciante/vuelve_pronto.wav",
                };
                _audio->play_random_effect_at(vuelve_pronto, 0.0f);
            }
            _shop_npc_id = -1;
        }
    }

    // Si me alejé del sacerdote con el que estaba hablando, despedida en 2 frases.
    if (_priest_npc_id != -1) {
        static constexpr float PRIEST_RANGE_TILES = 2.0f;
        const EntityDTO* priest_npc = nullptr;
        for (const auto& e : _last_entities)
            if (e.entity_id == _priest_npc_id) { priest_npc = &e; break; }

        bool left = (priest_npc == nullptr)
                  || (dist_to_player_tiles(priest_npc->pos_x, priest_npc->pos_y) > PRIEST_RANGE_TILES);
        if (left) {
            if (_audio) {
                static const std::vector<std::string> despedida = {
                    "assets/sounds/effects/npcs/sacerdote/ten_cuidado_ahi_fuera.wav",
                    "assets/sounds/effects/npcs/sacerdote/que_la_luz_guie_tu_camino.wav",
                };
                _audio->queue_speech_sequence(despedida, 0.0f);
            }
            _priest_npc_id = -1;
        }
    }

    if (snap.messages) {
        for (const auto& m : *snap.messages) {
            if (_chat) _chat->add_message(m.text);
            if (_audio && (m.text.rfind("Compraste ", 0) == 0 || m.text.rfind("Vendiste ", 0) == 0)) {
                static const std::vector<std::string> monedas = {
                    "assets/sounds/effects/economy/monedas.wav",
                };
                _audio->play_random_effect_at(monedas, 0.0f);
            }
        }
    }

    if (snap.is_ghost != 0 && !_was_ghost)
        play_player_death_sound(static_cast<uint16_t>(_player.tile_x),
                                 static_cast<uint16_t>(_player.tile_y));
    _was_ghost = (snap.is_ghost != 0);

    if (_audio && _level_initialized && snap.level > _last_level) {
        static const std::vector<std::string> subir_nivel = {
            "assets/sounds/effects/ui/sonido_al_subir_de_lvl.wav",
        };
        _audio->play_random_effect_at(subir_nivel, 0.0f);
    }
    _last_level = snap.level;
    _level_initialized = true;

    // Actualizar StatsPanel
    if (_stats) {
        uint8_t eq_weapon_item = 0;
        if (snap.equipped_wpn != 0xFF && snap.equipped_wpn < SnapshotDTO::INVENTORY_SIZE)
            eq_weapon_item = snap.inventory[snap.equipped_wpn];
        _stats->update(snap.hp, snap.max_hp,
                    snap.mp, snap.max_mp,
                    snap.gold, snap.level, snap.exp,
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
    render_deaths();
    render_obj_sup();
    render_spells();
    render_projectiles();

    int sw = _window.GetWidth();
    int sh = _window.GetHeight();

    if (_chat)      _chat->render(sw, sh);
    if (_stats)     _stats->render(sw, sh);
    if (_inventory) _inventory->render(sw, sh);
    if (_pos_label) _pos_label->render(sw, sh);

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
        { static_cast<uint8_t>(ItemId::LEATHER_ARMOR),          "assets/sprites/equipment/armor/clerigo_blanco.png" },
        { static_cast<uint8_t>(ItemId::CLERIC_BLACK_ARMOR),     "assets/sprites/equipment/armor/clerigo_negro.png" },
        { static_cast<uint8_t>(ItemId::MAGE_COMMON_ARMOR),      "assets/sprites/equipment/armor/mago_comun.png" },
        { static_cast<uint8_t>(ItemId::MAGE_ROYAL_ARMOR),       "assets/sprites/equipment/armor/mago_real.png" },
        { static_cast<uint8_t>(ItemId::PLATE_ARMOR),            "assets/sprites/equipment/armor/guerrero_ejecutor.png" },
        { static_cast<uint8_t>(ItemId::WARRIOR_EPIC_ARMOR),     "assets/sprites/equipment/armor/guerrero_epico.png" },
        { static_cast<uint8_t>(ItemId::PALADIN_MAGIC_ARMOR),    "assets/sprites/equipment/armor/paladin_magico.png" },
        { static_cast<uint8_t>(ItemId::PALADIN_ROYAL_ARMOR),    "assets/sprites/equipment/armor/paladin_real.png" },
    };

    // offsets indexados por [sprite_id de raza][dirección] (0=sin uso, 1=humano, 2=elfo, 3=enano, 4=gnomo)
    using RaceDirOffsets = std::array<std::array<int, 4>, 5>;

    struct HelmetInfo {
        std::string path;
        int src_x, src_y, src_w, src_h;
        RaceDirOffsets offset_y;
        RaceDirOffsets offset_x{};
    };
    // Cada fila de RaceDirOffsets es {sur, norte, oeste, este}.
    // Orden de filas: {sin_uso}, {humano}, {elfo}, {enano}, {gnomo}.
    static const std::unordered_map<uint8_t, HelmetInfo> helmet_info = {
        { static_cast<uint8_t>(ItemId::HOOD), { "assets/sprites/equipment/helmet/capucha.png", 0, 0, 17, 85 / 4,
            RaceDirOffsets{{ {0,0,0,0}, {-2,-2,-1,0}, {-2,-2,-1,-1}, {-2,-2,-1,-1}, {-2,-2,-6,-4} }},
            RaceDirOffsets{{ {0,0,0,0}, {0,0,0,0},    {0,0,0,-1},    {0,0,0,0},    {0,1,1,-1}    }} } },
        { static_cast<uint8_t>(ItemId::IRON_HELMET), { "assets/sprites/equipment/helmet/casco_de_hierro.png", 0, 0, 19, 110 / 4,
            RaceDirOffsets{{ {0,0,0,0}, {-8,-7,-5,-6}, {-8,-7,-6,-10}, {-9,-7,-6,-9}, {-10,-7,-11,-13} }},
            RaceDirOffsets{{ {0,0,0,0}, {0,0,2,-2},     {0,0,1,-2},     {0,0,1,-1},     {0,1,2,-2}    }} } },
        { static_cast<uint8_t>(ItemId::MAGIC_HAT), { "assets/sprites/equipment/helmet/sombrero_magico.png", 0, 0, 25, 113 / 4,
            RaceDirOffsets{{ {0,0,0,0}, {-12,-9,-10,-7}, {-12,-9,-12,-11}, {-12,-9,-12,-11}, {-12,-9,-16,-13} }},
            RaceDirOffsets{{ {0,0,0,0}, {-1,-1,1,-1},    {-1,-1,1,-1},     {-1,-1,1,0},     {-1,-1,1,-1}  }} } },
    };

    static const std::unordered_map<uint8_t, std::string> shield_paths = {
        { static_cast<uint8_t>(ItemId::TURTLE_SHIELD),  "assets/sprites/equipment/shield/escudo_tortuga.png" },
        { static_cast<uint8_t>(ItemId::IRON_SHIELD),    "assets/sprites/equipment/shield/escudo_hierro.png" },
        { static_cast<uint8_t>(ItemId::BOCA_SHIELD),    "assets/sprites/equipment/shield/escudo_boca.png" },
    };

    static const std::unordered_map<uint8_t, std::string> weapon_paths = {
        { static_cast<uint8_t>(ItemId::SWORD),              "assets/sprites/weapons/sword/espada.png" },
        { static_cast<uint8_t>(ItemId::AXE),                "assets/sprites/weapons/axe/hacha.png" },
        { static_cast<uint8_t>(ItemId::HAMMER),             "assets/sprites/weapons/hammer/martillo.png" },
        { static_cast<uint8_t>(ItemId::SIMPLE_BOW),         "assets/sprites/weapons/bow/arco_simple.png" },
        { static_cast<uint8_t>(ItemId::COMPOUND_BOW),       "assets/sprites/weapons/bow/arco_compuesto.png" },
        { static_cast<uint8_t>(ItemId::ELVEN_FLUTE),        "assets/sprites/weapons/flute/flauta_elfica.png" },
        { static_cast<uint8_t>(ItemId::ASH_STICK),          "assets/sprites/weapons/stick/vara_fresno.png" },
        { static_cast<uint8_t>(ItemId::NUDOSO_STAFF),       "assets/sprites/weapons/staff/baculo_nudoso.png" },
        { static_cast<uint8_t>(ItemId::GEMMED_STAFF),       "assets/sprites/weapons/staff/baculo_engarzado.png" },
    };

    for (const auto& e : _last_entities) {
        int screen_x = _camera.world_to_screen_x(static_cast<float>(e.pos_x * TILE_SIZE));
        int screen_y = _camera.world_to_screen_y(static_cast<float>(e.pos_y * TILE_SIZE));

        // ── Items en el suelo ──
        if (e.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR)) {
            if (e.sprite_id == static_cast<uint8_t>(ItemId::BLOOD_STAIN)) continue;

            static const std::unordered_map<uint8_t, std::vector<std::string>> item_variants = {
                {  1, { "assets/sprites/weapons/sword/espada.png" }},
                {  2, { "assets/sprites/weapons/axe/hacha.png" }},
                {  3, { "assets/sprites/weapons/hammer/martillo.png" }},
                {  4, { "assets/sprites/weapons/bow/arco_simple.png" }},
                {  5, { "assets/sprites/weapons/bow/arco_compuesto.png" }},
                {  6, { "assets/sprites/weapons/flute/flauta_elfica.png" }},
                {  7, { "assets/sprites/weapons/staff/baculo_engarzado.png" }},
                {  8, { "assets/sprites/weapons/stick/vara_fresno.png" }},
                {  9, { "assets/sprites/weapons/staff/baculo_nudoso.png" }},
                { 10, { "assets/sprites/equipment/armor/clerigo_blanco.png", "assets/sprites/equipment/armor/clerigo_negro.png", "assets/sprites/equipment/armor/mago_comun.png", "assets/sprites/equipment/armor/mago_real.png" }},
                { 11, { "assets/sprites/equipment/armor/guerrero_ejecutor.png", "assets/sprites/equipment/armor/guerrero_epico.png", "assets/sprites/equipment/armor/paladin_magico.png", "assets/sprites/equipment/armor/paladin_real.png" }},
                { 30, { "assets/sprites/equipment/shield/escudo_tortuga.png" }},
                { 31, { "assets/sprites/equipment/shield/escudo_hierro.png", "assets/sprites/equipment/shield/escudo_boca.png" }},
                { static_cast<uint8_t>(ItemId::BLOOD_STAIN), { "assets/sprites/stage/sangre_5.png" }},
                { static_cast<uint8_t>(ItemId::GOLD_PILE), { "assets/sprites/items/oro.png" }},
                { static_cast<uint8_t>(ItemId::HEALTH_POTION), { "assets/sprites/items/pocion_vida.png" }},
                { static_cast<uint8_t>(ItemId::MANA_POTION), { "assets/sprites/items/pocion_mana.png" }},
            };

            std::string path = "assets/sprites/items/oro.png";
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
            
            continue; // Los items en el piso no llevan barra de vida
        }

        Direction dir = static_cast<Direction>(e.direction);
        bool moving = false;

        if (e.entity_id == _my_entity_id) {
            screen_x = _camera.world_to_screen_x(_player.pixel_x());
            screen_y = _camera.world_to_screen_y(_player.pixel_y());
            moving = _player.is_moving();
        }

        // ── NPC / Criaturas hostiles ──
        if (e.entity_type == static_cast<uint8_t>(EntityType::NPC)) {
            struct NpcSheet { std::string path; int cols, rows, frame_w, frame_h; };
            static const std::unordered_map<uint8_t, std::vector<NpcSheet>> npc_sheets = {
                { 1, { { "assets/npcs/goblin/duende_ladron.png",      7, 4,  25,  32 },
                       { "assets/npcs/goblin/duende_zombie_real.png", 8, 4,  64,  67 }, }},
                { 2, { { "assets/npcs/skeleton/esqueleto_magico.png", 4, 4, 64, 64 },
                       { "assets/npcs/skeleton/esqueleto_muerte.png", 5, 3, 25, 50 },
                       { "assets/npcs/skeleton/esqueleto_oscuro.png", 7, 3, 24, 32 }, }},
                { 3, { { "assets/npcs/zombie/zombie_comun.png",   5, 4,  24,  50 },
                       { "assets/npcs/zombie/zombie_magico.png",  5, 4,  27,  50 },
                       { "assets/npcs/zombie/zombie_poseido.png", 4, 4,  89, 106 }, }},
                { 4, { { "assets/npcs/spider/araña_comun.png",       8, 4,  64,  64 },
                       { "assets/npcs/spider/araña_endurecida.png",  5, 4, 192, 192 }, }},
                { 5, { { "assets/npcs/orc/orco_errante.png", 6, 4,  56, 100 },
                       { "assets/npcs/orc/orco_fuego.png",   5, 4,  57,  99 }, }},
                { 6, { { "assets/npcs/golem/golem_moribundo.png",  4, 4,  74,  50 },
                       { "assets/npcs/golem/golem_reforzado.png",  6, 4,  79, 128 },
                       { "assets/npcs/golem/golem_tierra.png",     6, 4, 140, 180 }, }},
                // NPCs de servicio
                { 7, { { "assets/npcs/comerciante/comerciante_ciudad.png", 1, 1, 31, 42 },
                       { "assets/npcs/comerciante/comerciante_pueblo.png", 1, 1, 23, 45 }, }},
                { 8, { { "assets/npcs/banquero/banquero_ciudad.png",       1, 1, 23, 56 },
                       { "assets/npcs/banquero/banquero_pueblo.png",       1, 1, 27, 54 }, }},
                { 9, { { "assets/npcs/sacerdote/sacerdote_ciudad.png",     1, 1, 25, 55 },
                       { "assets/npcs/sacerdote/sacerdote_pueblo.png",     1, 1, 27, 54 }, }},
            };

            bool is_service_npc = (e.sprite_id >= 7);  // MERCHANT=7, BANKER=8, PRIEST=9
            bool is_pueblo = is_service_npc && (e.pos_y > 50);
            float scale = is_service_npc ? 1.0f : 1.5f;
            int draw_offset_y = !is_service_npc ? 0
                               : (e.sprite_id == 9 /*PRIEST*/) ? (is_pueblo ? 18 : 0) : 14;

            auto sit = npc_sheets.find(e.sprite_id);
            if (sit != npc_sheets.end() && !sit->second.empty()) {
                const auto& sheets = sit->second;
                int sheet_idx = is_service_npc
                    ? (is_pueblo ? 1 : 0)
                    : static_cast<int>(e.entity_id % sheets.size());
                const NpcSheet& s = sheets[sheet_idx];

                // Dibujamos el NPC
                SpriteBounds bounds = _anim.render_npc(_renderer, _assets,
                                 s.path, s.cols, s.rows, s.frame_w, s.frame_h,
                                 dir, screen_x, screen_y,
                                 _current_tick, moving, scale, draw_offset_y);
                render_entity_healthbar(e, bounds);
            }
            continue; // El continue ahora sí ocurre después de renderizar el healthbar
        }

        // ── Otros Jugadores Humanos ──
        EquipVisual equip{};
        const EquipVisual* equip_ptr = &equip;

        uint8_t item_weapon, item_armor, item_helmet, item_shield;
        if (e.entity_id == _my_entity_id) {
            item_weapon = (_eq_wpn  != 0xFF && _eq_wpn  < SnapshotDTO::INVENTORY_SIZE) ? _inv[_eq_wpn]  : 0;
            item_armor  = (_eq_arm  != 0xFF && _eq_arm  < SnapshotDTO::INVENTORY_SIZE) ? _inv[_eq_arm]  : 0;
            item_helmet = (_eq_helm != 0xFF && _eq_helm < SnapshotDTO::INVENTORY_SIZE) ? _inv[_eq_helm] : 0;
            item_shield = (_eq_shld != 0xFF && _eq_shld < SnapshotDTO::INVENTORY_SIZE) ? _inv[_eq_shld] : 0;
        } else {
            item_weapon = e.equipped_weapon;
            item_armor  = e.equipped_armor;
            item_helmet = e.equipped_helmet;
            item_shield = e.equipped_shield;
        }

        if (item_weapon != 0) {
            auto wit = weapon_paths.find(item_weapon);
            if (wit != weapon_paths.end()) equip.weapon_path = wit->second;
        }
        if (item_armor != 0) {
            auto ait = armor_paths.find(item_armor);
            if (ait != armor_paths.end()) equip.armor_path = ait->second;
        }
        if (item_helmet != 0) {
            auto hit = helmet_info.find(item_helmet);
            if (hit != helmet_info.end()) {
                equip.helmet_path = hit->second.path;
                equip.helmet_src_x = hit->second.src_x;
                equip.helmet_src_y = hit->second.src_y;
                equip.helmet_src_w = hit->second.src_w;
                equip.helmet_src_h = hit->second.src_h;
                int race = (e.sprite_id >= 1 && e.sprite_id <= 4) ? e.sprite_id : 1;
                for (int d = 0; d < 4; d++) {
                    equip.helmet_offset_y[d] = hit->second.offset_y[race][d];
                    equip.helmet_offset_x[d] = hit->second.offset_x[race][d];
                }
            }
        }
        if (item_shield != 0) {
            auto sit = shield_paths.find(item_shield);
            if (sit != shield_paths.end()) equip.shield_path = sit->second;
        }

        const SpriteEntry& sprite = _sprite_config.get(e.sprite_id);
        SpriteBounds bounds = _anim.render(_renderer, _assets,
                     sprite.body_path, sprite.head_path,
                     e.sprite_id, dir,
                     screen_x, screen_y,
                     _current_tick, moving,
                     equip_ptr, e.is_ghost != 0);

        // Renderizar barra para los demás jugadores (aquellos que no tengan "continue" previo)
        if (e.entity_id != _my_entity_id) {
            render_entity_healthbar(e, bounds);
        }
    }
}

void GameLoop::render_entity_healthbar(const EntityDTO& entity, const SpriteBounds& bounds) {
    if (entity.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR)) return;

    const bool is_service_npc = (entity.entity_type == static_cast<uint8_t>(EntityType::NPC))
                                && (entity.sprite_id >= 7);  // MERCHANT=7, BANKER=8, PRIEST=9

    const int margin = 6;  // separación entre overlay y punta del sprite
    int text_anchor_y = bounds.top_y - margin;

    if (!is_service_npc) {
        // El ancho de la barra escala con el sprite real en pantalla.
        const int bar_w = std::clamp(bounds.width, 24, 50);
        const int bar_h = 4;

        // Posición de la barra, centrada sobre el sprite.
        int bar_x = bounds.center_x - bar_w / 2;
        int bar_y = bounds.top_y - bar_h - margin;
        text_anchor_y = bar_y;

        // Renderizar barra de fondo
        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer.Get(), 30, 30, 30, 220);
        SDL_Rect bg_rect{ bar_x, bar_y, bar_w, bar_h };
        SDL_RenderFillRect(_renderer.Get(), &bg_rect);

        // Renderizar barra de vida
        int hp_bar_w = static_cast<int>(bar_w * entity.hp_pct / 100.0f);
        if (hp_bar_w > 0) {
            SDL_Color hp_color;
            if (entity.hp_pct > 60) {
                hp_color = { 100, 220, 100, 255 };
            } else if (entity.hp_pct > 30) {
                hp_color = { 255, 220, 50, 255 };
            } else {
                hp_color = { 220, 80, 80, 255 };
            }
            SDL_SetRenderDrawColor(_renderer.Get(), hp_color.r, hp_color.g, hp_color.b, hp_color.a);
            SDL_Rect hp_rect{ bar_x, bar_y, hp_bar_w, bar_h };
            SDL_RenderFillRect(_renderer.Get(), &hp_rect);
        }

        // Borde de la barra
        SDL_SetRenderDrawColor(_renderer.Get(), 200, 200, 200, 255);
        SDL_RenderDrawRect(_renderer.Get(), &bg_rect);
    }

    // Los jugadores muestran nombre sobre la barra; los NPCs de servicio muestran solo nombre
    // arriba de la cabeza, sin barra de vida.
    const bool show_name = ((entity.entity_type == static_cast<uint8_t>(EntityType::PLAYER))
                            || is_service_npc)
                           && !entity.username.empty();
    if (!show_name) return;

    if (!_small_font) {
        _small_font = TTF_OpenFont(CHAT_FONT_PATH, 10);
    }

    if (_small_font) {
        SDL_Color name_color = { 200, 220, 255, 255 };
        SDL_Surface* surf = TTF_RenderUTF8_Blended(_small_font, entity.username.c_str(), name_color);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
            if (tex) {
                bool is_pueblo_priest = (entity.sprite_id == 9) && (entity.pos_y > 50);
                int service_name_offset = (entity.sprite_id == 9 /*PRIEST*/)
                                               ? (is_pueblo_priest ? 2 : -10)
                                               : 2;
                int name_y = is_service_npc
                                 ? bounds.top_y + service_name_offset
                                 : text_anchor_y - surf->h - 2;
                int name_x = bounds.center_x - surf->w / 2;
                SDL_Rect dst{ name_x, name_y, surf->w, surf->h };
                SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }
}

void GameLoop::handle_mouse_click(int mouse_x, int mouse_y) {
    if (!_command_queue) return;
    if (mouse_x >= _window.GetWidth() - StatsPanel::PANEL_W) return;

    int world_x = mouse_x - _camera.tile_to_screen_x(0);
    int world_y = mouse_y - _camera.tile_to_screen_y(0);
    int tile_x  = world_x / TILE_SIZE;
    int tile_y  = world_y / TILE_SIZE;

    for (const auto& e : _last_entities) {
        if (e.entity_id == _my_entity_id) continue;
        if (e.pos_x != tile_x || e.pos_y != tile_y) continue;
        // Los items en el piso (oro, sangre, drops) no son objetivos válidos:
        // si comparten tile con un NPC/jugador, no deben tapar el click sobre él.
        if (e.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR)) continue;

        bool is_service_npc = (e.entity_type == static_cast<uint8_t>(EntityType::NPC))
                              && (e.sprite_id >= 7);  // MERCHANT=7, BANKER=8, PRIEST=9

        if (is_service_npc) {
            // Interactuar con NPC de servicio
            _command_queue->push(Command::npc_interact(e.entity_id));
            _chat->add_message("Hablando con " + e.username + "...");
            if (static_cast<NpcId>(e.sprite_id) == NpcId::MERCHANT) {
                // Si ya estoy hablando con este mismo comerciante, no repetir
                // el saludo (solo re-enviar la interacción/mensaje de chat).
                bool already_talking = (_shop_npc_id == static_cast<int32_t>(e.entity_id));
                _shop_npc_id = e.entity_id;
                if (_audio && !already_talking) {
                    static const std::vector<std::string> saludo = {
                        "assets/sounds/effects/npcs/comerciante/buenos_dias.wav",
                        "assets/sounds/effects/npcs/comerciante/bienvenido_a_mi_tienda.wav",
                        "assets/sounds/effects/npcs/comerciante/como_puedo_ayudar.wav",
                    };
                    _audio->queue_speech_sequence(saludo, dist_to_player_tiles(e.pos_x, e.pos_y));
                }
            } else if (static_cast<NpcId>(e.sprite_id) == NpcId::BANKER) {
                // si ya estoy hablando con este mismo banquero, no repetir el saludo.
                bool already_talking = (_bank_npc_id == static_cast<int32_t>(e.entity_id));
                _bank_npc_id = e.entity_id;
                if (_audio && !already_talking) {
                    static const std::vector<std::string> saludo = {
                        "assets/sounds/effects/npcs/banquero/bienvenido_al_banco.wav",
                        "assets/sounds/effects/npcs/banquero/que_transaccion_desea_realizar_hoy.wav",
                    };
                    _audio->queue_speech_sequence(saludo, dist_to_player_tiles(e.pos_x, e.pos_y));
                }
            } else if (static_cast<NpcId>(e.sprite_id) == NpcId::PRIEST) {
                bool already_talking = (_priest_npc_id == static_cast<int32_t>(e.entity_id));
                _priest_npc_id = e.entity_id;
                if (_audio && !already_talking) {
                    static const std::vector<std::string> saludo = {
                        "assets/sounds/effects/npcs/sacerdote/oigo_tus_plegarias.wav",
                    };
                    _audio->play_random_effect_at(saludo, dist_to_player_tiles(e.pos_x, e.pos_y));
                }
            }
        } else if (_stats && _stats->cast_mode_active() && _stats->selected_spell() != 0) {
            uint8_t spell = _stats->selected_spell();
            _command_queue->push(Command::cast_spell(e.entity_id, spell));
            spawn_spell_effect(spell, e.pos_x, e.pos_y);
            play_spell_sound(spell, e.pos_x, e.pos_y);
            _chat->add_message("Lanzando hechizo a " + (e.username.empty()
                ? std::string("#") + std::to_string(e.entity_id) : e.username));
        } else {
            _command_queue->push(Command::attack(e.entity_id));
            _chat->add_message("Atacando a " + (e.username.empty()
                ? std::string("#") + std::to_string(e.entity_id) : e.username));

            uint8_t my_weapon = (_eq_wpn != 0xFF && _eq_wpn < SnapshotDTO::INVENTORY_SIZE)
                                 ? _inv[_eq_wpn] : 0;
            if (weapon_is_ranged(my_weapon)) {
                spawn_projectile(static_cast<uint16_t>(_player.tile_x),
                                 static_cast<uint16_t>(_player.tile_y),
                                 e.pos_x, e.pos_y,
                                 weapon_is_magic(my_weapon));
            }
            play_attack_sound(my_weapon, e.pos_x, e.pos_y);
        }
        return;
    }
}
