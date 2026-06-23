#ifndef WORLD_SPAWNER_H
#define WORLD_SPAWNER_H

#include <cstdint>
#include <random>
#include <vector>
#include "../Npc.h"
#include "ZoneTypes.h"

class WorldCollision;
class WorldNpcs;

// Zonas de spawn de NPCs (con cooldown y cap por zona y global) y zonas
// seguras donde no se puede atacar ni los NPCs persiguen.
class WorldSpawner {
public:
    WorldSpawner(WorldCollision& c, std::mt19937& rng)
        : collision(c), rng(rng) {}

    void add_zone(const SpawnZone& z)     { zones.push_back(z); }
    void add_safe_zone(const SafeZone& z) { safe_zones.push_back(z); }

    void set_global_cap(uint16_t cap) { global_cap = cap; }
    uint16_t global_cap_value() const { return global_cap; }

    std::vector<SpawnZone>&       zones_mut()       { return zones; }
    const std::vector<SpawnZone>& zones_view() const { return zones; }

    // Tick: baja cooldowns y spawnea NPCs respetando caps.
    // Devuelve los spawns pedidos (zona_id, type, x, y) para que WorldNpcs los materialice.
    struct PendingSpawn { uint8_t zone_id; NpcId type; uint16_t x; uint16_t y; };
    std::vector<PendingSpawn> tick(uint16_t total_alive);

    bool in_safe_zone(uint16_t x, uint16_t y) const;

private:
    WorldCollision& collision;
    std::mt19937&   rng;
    std::vector<SpawnZone> zones;
    std::vector<SafeZone>  safe_zones;
    uint16_t global_cap = 15;

    bool pick_free_cell(const SpawnZone& z, uint16_t& out_x, uint16_t& out_y);
};

#endif
