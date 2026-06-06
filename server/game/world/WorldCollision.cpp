#include "WorldCollision.h"
#include "WorldPlayers.h"
#include "WorldNpcs.h"

WorldCollision::WorldCollision(uint16_t w, uint16_t h) : width(w), height(h) {
    for (uint16_t x = 0; x < width; ++x)
        for (uint16_t y = 0; y < height; ++y)
            occupied[{x, y}] = false;
}

bool WorldCollision::is_occupied(uint16_t x, uint16_t y) const {
    auto it = occupied.find({x, y});
    return it != occupied.end() && it->second;
}

void WorldCollision::update(uint16_t x, uint16_t y, bool occ) {
    if (!in_bounds(x, y)) return;
    occupied[{x, y}] = occ;
}

void WorldCollision::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occ) {
    update(pos.first, pos.second, occ);
}

void WorldCollision::revisar(const WorldPlayers& players, const WorldNpcs& npcs) {
    for (const auto& [id, p] : players.all())
        update(p.pos_x, p.pos_y, true);
    for (const auto& n : npcs.all())
        update(n.pos_x, n.pos_y, true);
}
