#include "WorldCollision.h"

WorldCollision::WorldCollision(uint16_t w, uint16_t h, std::vector<uint8_t> occ)
    : width(w), height(h), occupied(std::move(occ)) {}

size_t WorldCollision::index(uint16_t x, uint16_t y) const {
    return static_cast<size_t>(y) * width + x;
}

uint16_t WorldCollision::get_width() const {
    return width;
}

uint16_t WorldCollision::get_height() const {
    return height;
}

bool WorldCollision::in_bounds(uint16_t x, uint16_t y) const {
    return x < width && y < height;
}

bool WorldCollision::is_occupied(uint16_t x, uint16_t y) const {
    if (!in_bounds(x, y)) return false;
    return occupied[index(x, y)] > 0;
}

void WorldCollision::update(uint16_t x, uint16_t y, bool occ) {
    if (!in_bounds(x, y)) return;
    size_t idx = index(x, y);
    if (occ) {
        occupied[idx]++;
    } else {
        if (occupied[idx] > 0) occupied[idx]--;
    }
}

void WorldCollision::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occ) {
    update(pos.first, pos.second, occ);
}