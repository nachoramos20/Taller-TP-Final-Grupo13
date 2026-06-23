#include "Mazmorra.h"

#include "../../../common/protocol/protocol.h"
#include "../Equations.h"

#include "WorldItems.h"
#include "WorldNpcs.h"

Mazmorra::Mazmorra(WorldNpcs& npcs, WorldItems& items, uint16_t x1, uint16_t y1, uint16_t x2,
                   uint16_t y2):
        npcs_(npcs), items_(items), x1_(x1), y1_(y1), x2_(x2), y2_(y2) {}

void Mazmorra::add_spawn(uint16_t x, uint16_t y, const std::vector<NpcId>& allowed_types) {
    if (allowed_types.empty())
        return;
    int index = Equations::rand_range(0, static_cast<int>(allowed_types.size()) - 1);
    spawns_.push_back({allowed_types[static_cast<size_t>(index)], x, y});
}

bool Mazmorra::in_mazmorra(uint16_t x, uint16_t y) const {
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

void Mazmorra::add_gold(uint16_t x, uint16_t y, uint32_t amount) {
    gold_spawns_.push_back({x, y, amount});
}

void Mazmorra::respawn() {
    if (activa())
        return;
    npcs_.kill_all_in_zone(x1_, y1_, x2_, y2_);
    items_.remove_in_zone(x1_, y1_, x2_, y2_);
    for (const MazmorraSpawnPoint& spawn_point: spawns_) {
        npcs_.spawn(spawn_point.type, spawn_point.x, spawn_point.y);
    }
    for (const GoldSpawnPoint& gp: gold_spawns_) {
        items_.add(static_cast<uint8_t>(ItemId::GOLD_PILE), gp.x, gp.y, gp.amount, 0);
    }
}
