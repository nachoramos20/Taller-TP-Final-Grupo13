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

class Mazmorra {
public:
    Mazmorra(WorldNpcs& npcs, WorldItems& items,
             uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void add_spawn(NpcId type, uint16_t x, uint16_t y);
    void respawn();

    bool activa() const { return activa_; }
    void set_activa(bool val) { activa_ = val; }
    bool in_mazmorra(uint16_t x, uint16_t y);

private:
    WorldNpcs& npcs_;
    WorldItems& items_;
    uint16_t x1_, y1_, x2_, y2_;
    bool activa_ = false;
    std::vector<MazmorraSpawnPoint> spawns_;
};

#endif