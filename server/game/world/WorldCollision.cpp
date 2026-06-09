#include "WorldCollision.h"
#include "WorldPlayers.h"
#include "WorldNpcs.h"

#include <algorithm>

WorldCollision::WorldCollision(uint16_t w, uint16_t h)
    : width(w), height(h) {
    occupied.resize(static_cast<size_t>(width) * height, 0);
}
size_t WorldCollision::index(uint16_t x, uint16_t y) const {
    return static_cast<size_t>(y) * width + x;
}

bool WorldCollision::in_bounds(uint16_t x, uint16_t y) const {
    return x < width && y < height;
}


bool WorldCollision::is_occupied(uint16_t x, uint16_t y) const {
    if (!in_bounds(x, y)) return false;
    return occupied[index(x, y)] == 1;
}

void WorldCollision::update(uint16_t x, uint16_t y, bool occ) {
    if (!in_bounds(x, y)) return;
    occupied[index(x, y)] = occ ? 1 : 0;
}

void WorldCollision::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occ) {
    update(pos.first, pos.second, occ);
}

void WorldCollision::revisar(const WorldPlayers& players, const WorldNpcs& npcs) {
    std::fill(occupied.begin(), occupied.end(), 0);
    for (const auto& [id, p] : players.all())
        update(p.pos_x, p.pos_y, true);
    for (const auto& n : npcs.all())
        update(n.pos_x, n.pos_y, true);
}