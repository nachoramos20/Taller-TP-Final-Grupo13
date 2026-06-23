#pragma once

#include <cstdint>
#include <vector>
#include "../Npc.h"

// Zona rectangular donde el WorldSpawner mantiene una población de NPCs
// hostiles, regenerándolos con cooldown hasta un tope (ver WorldSpawner::tick).
struct SpawnZone {
    uint16_t x1, y1, x2, y2;             // rect inclusivo
    std::vector<NpcId> allowed_types;
    uint16_t max_alive         = 5;      // tope por zona
    uint16_t spawn_every_ticks = 30 * 8; // 8 seg @ 30Hz
    uint16_t cooldown          = 0;      // se decrementa cada tick
    uint16_t alive_count       = 0;      // se actualiza desde WorldNpcs
};

// Zona rectangular (ciudad/pueblo) donde el server rechaza ataques a
// distancia/hechizos y los NPCs hostiles no atacan (ver World::in_safe_zone).
struct SafeZone {
    uint16_t x1, y1, x2, y2; // rect inclusivo
};
