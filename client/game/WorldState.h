#pragma once

#include <cstdint>
#include <string>

#include <unordered_map>
#include <vector>

#include "../../common/protocol/dtos.h"
#include "../../common/MapaDTO.h"
#include "PlayerState.h"

// Interpolación de movimiento de una entidad (NPC u otro jugador) entre la
// posición de tile que tenía y la nueva que llegó en el snapshot, para que
// se vea deslizar en vez de saltar de golpe (igual que el jugador propio
// vía PlayerState, pero indexado por entity_id en vez de ser uno solo).
struct EntityMotion {
    float from_x = 0.0f, from_y = 0.0f;
    float to_x   = 0.0f, to_y   = 0.0f;
    float progress = 1.0f;  // 0 (recién arrancó) .. 1 (ya llegó)
};

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

    // Interpolación de movimiento por entidad. No incluye
    // al jugador propio, que ya interpola por su cuenta vía PlayerState.
    std::unordered_map<uint16_t, EntityMotion> entity_motion;

    bool    was_ghost = false;
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

// Distancia Manhattan usada para validar rango de hechizos y armas a
// distancia ANTES de spawnear el efecto visual/sonido. La euclidiana de
// dist_to_player_tiles da una distancia menor en diagonal, lo que hacía que
// el cliente creyera estar en rango cuando el servidor ya lo rechazaba.
int manhattan_dist_to_player_tiles(const PlayerState& player, uint16_t x, uint16_t y);
float distance_to_nearest_water_tile(const WorldState& state, const PlayerState& player);
bool  is_floor_grass(const WorldState& state, uint16_t x, uint16_t y);
bool  is_floor_dirt(const WorldState& state, uint16_t x, uint16_t y);
bool  is_floor_city_stone(const WorldState& state, uint16_t x, uint16_t y);
bool  is_in_forest_zone(uint16_t x, uint16_t y);
float distance_to_cemetery_zone(int x, int y);

// Item id del arma equipada por el jugador propio (0 si no tiene nada equipado).
uint8_t own_weapon_item(const WorldState& state);

// Actualiza el destino de interpolación de cada entidad del snapshot nuevo
// y descarta las que ya no existen. Llamar antes de pisar
// state.entities con el snapshot nuevo.
void update_entity_motion(WorldState& state, const std::vector<EntityDTO>& new_entities);

// Avanza el progreso de todas las interpolaciones en curso. Llamar todos los
// frames con el dt real (no por snapshot).
void advance_entity_motion(WorldState& state, float dt);

// Posición interpolada en píxeles de una entidad (no aplica al jugador
// propio, que se dibuja con PlayerState::pixel_x/y).
float entity_pixel_x(const WorldState& state, const EntityDTO& e);
float entity_pixel_y(const WorldState& state, const EntityDTO& e);

void spawn_spell_effect(WorldState& state, uint8_t spell_id, uint16_t pos_x, uint16_t pos_y);
void spawn_projectile(WorldState& state, uint16_t from_x, uint16_t from_y,
                       uint16_t to_x, uint16_t to_y, bool is_magic);
