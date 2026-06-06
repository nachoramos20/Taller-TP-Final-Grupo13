#ifndef WORLD_NPCS_H
#define WORLD_NPCS_H

#include <cstdint>
#include <random>
#include <vector>

#include "../Npc.h"
#include "IdAllocator.h"

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

    int rand_range(int lo, int hi);
public:
    WorldNpcs(WorldCollision& c, WorldPlayers& p, WorldItems& i,
              WorldChat& ch, WorldClans& cl, IdAllocator& a, std::mt19937& r)
        : collision(c), players(p), items(i), chat(ch), clans(cl), id_alloc(a), rng(r) {}

    const std::vector<NpcData>& all() const { return npcs; }

    void spawn(NpcId type, uint16_t x, uint16_t y);
    void tick();
    NpcData* find(uint16_t id);
};

#endif
