#include "Mazmorra.h"
#include "WorldNpcs.h"
#include "WorldItems.h"

Mazmorra::Mazmorra(WorldNpcs& npcs, WorldItems& items,
                   uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
    : npcs_(npcs), items_(items),
      x1_(x1), y1_(y1), x2_(x2), y2_(y2) {}

void Mazmorra::add_spawn(NpcId type, uint16_t x, uint16_t y) {
    spawns_.push_back({type, x, y});
}

bool Mazmorra::in_mazmorra(uint16_t x, uint16_t y) {
    return x >= x1_ && x <= x2_ && y >= y1_ && y <= y2_;
}

void Mazmorra::player_entered() {
    if (player_count_ == 0) {
        respawn();
    }
    player_count_++;
}

void Mazmorra::player_left() {
    if (player_count_ > 0) {
        player_count_--;
    }
}

void Mazmorra::respawn() {
    if (activa()) return;
    npcs_.kill_all_in_zone(x1_, y1_, x2_, y2_);
    items_.remove_in_zone(x1_, y1_, x2_, y2_);
    for (const MazmorraSpawnPoint& spawn_point : spawns_) {
        npcs_.spawn(spawn_point.type, spawn_point.x, spawn_point.y);
    }
}
