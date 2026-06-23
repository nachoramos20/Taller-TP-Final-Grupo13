#ifndef MAZMORRA_H
#define MAZMORRA_H

#include <cstdint>
#include <vector>
#include "../Npc.h"

class WorldNpcs;
class WorldItems;

struct MazmorraSpawnPoint {
    NpcId type;
    uint16_t x, y;
};

struct GoldSpawnPoint {
    uint16_t x, y;
    uint32_t amount;
};

class Mazmorra {
public:
    Mazmorra(WorldNpcs& npcs, WorldItems& items,
             uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void add_spawn(uint16_t x, uint16_t y);
    void add_gold(uint16_t x, uint16_t y, uint32_t amount);
    void respawn();

    bool activa() const { return player_count_ > 0; }
    bool in_mazmorra(uint16_t x, uint16_t y);
    void player_entered();
    void player_left();

private:
    WorldNpcs& npcs_;
    WorldItems& items_;
    uint16_t x1_, y1_, x2_, y2_;
    uint16_t player_count_ = 0;
    std::vector<MazmorraSpawnPoint> spawns_;
    std::vector<GoldSpawnPoint> gold_spawns_;
};

#endif