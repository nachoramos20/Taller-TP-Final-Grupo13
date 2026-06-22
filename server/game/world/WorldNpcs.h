#ifndef WORLD_NPCS_H
#define WORLD_NPCS_H

#include <cstdint>
#include <random>
#include <vector>

#include "../Npc.h"
#include "IdAllocator.h"
#include "WorldSpawner.h"

class WorldCollision;
class WorldPlayers;
class WorldItems;
class WorldChat;
class WorldClans;

class WorldNpcs {
private:
    std::vector<NpcData> npcs;

    WorldCollision& collision;
    WorldPlayers&   players;
    WorldItems&     items;
    WorldChat&      chat;
    WorldClans&     clans;
    IdAllocator&    id_alloc;
    std::mt19937&   rng;
    WorldSpawner&   spawner;

    int rand_range(int lo, int hi);
    void spawn_internal(NpcId type, uint16_t x, uint16_t y, uint8_t zone_id);
    void drop_npc_loot(const NpcData& npc);
    uint32_t current_tick = 0;

public:
    WorldNpcs(WorldCollision& c, WorldPlayers& p, WorldItems& i,
              WorldChat& ch, WorldClans& cl, IdAllocator& a,
              std::mt19937& r, WorldSpawner& sp)
        : collision(c), players(p), items(i), chat(ch), clans(cl),
          id_alloc(a), rng(r), spawner(sp) {}

    const std::vector<NpcData>& all() const { return npcs; }
    std::vector<NpcData>&       all_mutable() { return npcs; }

    void spawn(NpcId type, uint16_t x, uint16_t y);  // legacy/manual
    void spawn_with_zone(NpcId type, uint16_t x, uint16_t y, uint8_t zone_id);
    void tick(uint32_t current_tick);
    NpcData* find(uint16_t id);
    void cleanup_dead();
};

#endif
