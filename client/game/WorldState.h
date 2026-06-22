#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/protocol/dtos.h"
#include "../../common/MapaDTO.h"
#include "PlayerState.h"

// Efecto visual de hechizo (solo cliente)
struct SpellEffect {
    uint8_t  spell_id;
    uint16_t pos_x, pos_y;   // posición del objetivo en tiles
    uint32_t start_tick;
    int      sheet_cols;
    int      frame_w, frame_h;
    std::vector<int> frame_indices;
    std::string path;
};

// Animación de proyectil para ataques a distancia (solo cliente, sin sprite
// propio: se dibuja como una marca viajando del atacante al objetivo).
struct Projectile {
    uint16_t from_x, from_y;
    uint16_t to_x, to_y;
    uint32_t start_tick;
    bool     is_magic;  // color distinto para distinguir flecha de hechizo
};

// Animación de muerte de NPC
struct DeathEffect {
    uint16_t pos_x, pos_y;
    uint32_t start_ms;   // SDL_GetTicks() al crear el efecto
};

// Estado del mundo derivado de los snapshots del servidor: entidades
// visibles, equipo/inventario propio, efectos visuales en curso, mapa
// cargado, y los NPCs de servicio con los que se está interactuando.
// `SnapshotProcessor` lo actualiza con cada snapshot; `WorldRenderer` lo lee
// para dibujar, y `PlayerActionController` lo lee/empuja efectos nuevos al
// resolver clicks y comandos de chat.
struct WorldState {
    std::vector<EntityDTO> entities;
    uint16_t my_entity_id = 0;
    uint32_t current_tick = 0;

    bool    was_ghost = false;
    bool    was_meditating = false;
    uint8_t last_level = 0;
    bool    level_initialized = false;
    bool    spawned = false;  // primer snapshot recibido 

    // Ids de los NPC de servicio con los que se está interactuando
    // actualmente (-1 = ninguno).
    int32_t shop_npc_id   = -1;
    int32_t bank_npc_id   = -1;
    int32_t priest_npc_id = -1;

    // Equipo del jugador propio (slots de inventario, 0xFF = vacío)
    uint8_t inventory[SnapshotDTO::INVENTORY_SIZE]{};
    uint8_t eq_weapon = 0xFF, eq_armor = 0xFF, eq_helmet = 0xFF, eq_shield = 0xFF;

    std::vector<SpellEffect> spell_effects;
    std::vector<Projectile>  projectiles;
    std::vector<DeathEffect> death_effects;

    MapaDTO map;
    bool    map_loaded = false;
};

float dist_to_player_tiles(const PlayerState& player, uint16_t x, uint16_t y);
float distance_to_nearest_water_tile(const WorldState& state, const PlayerState& player);
bool  is_floor_grass(const WorldState& state, uint16_t x, uint16_t y);
bool  is_floor_dirt(const WorldState& state, uint16_t x, uint16_t y);
bool  is_floor_city_stone(const WorldState& state, uint16_t x, uint16_t y);
bool  is_in_forest_zone(uint16_t x, uint16_t y);
float distance_to_cemetery_zone(int x, int y);

// Item id del arma equipada por el jugador propio (0 si no tiene nada equipado).
uint8_t own_weapon_item(const WorldState& state);

void spawn_spell_effect(WorldState& state, uint8_t spell_id, uint16_t pos_x, uint16_t pos_y);
void spawn_projectile(WorldState& state, uint16_t from_x, uint16_t from_y,
                       uint16_t to_x, uint16_t to_y, bool is_magic);
