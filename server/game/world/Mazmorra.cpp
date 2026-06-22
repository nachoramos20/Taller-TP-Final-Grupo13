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

void Mazmorra::respawn() {
    // 1) Matar todos los NPCs dentro del rectángulo de la mazmorra
    npcs_.kill_all_in_zone(x1_, y1_, x2_, y2_);

    // 2) Limpiar items del suelo dentro del rectángulo
    items_.remove_in_zone(x1_, y1_, x2_, y2_);

    // 3) Spawnear todos los puntos definidos
    for (const MazmorraSpawnPoint& sp : spawns_) {
        npcs_.spawn(sp.type, sp.x, sp.y);
    }
}
