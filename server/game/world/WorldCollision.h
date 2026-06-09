#ifndef WORLD_COLLISION_H
#define WORLD_COLLISION_H

#include <cstdint>
#include <utility>
#include <vector>

class WorldPlayers;
class WorldNpcs;

class WorldCollision {
private:
    uint16_t width;
    uint16_t height;
    std::vector<uint8_t> occupied;

    inline size_t index(uint16_t x, uint16_t y) const;

public:
    WorldCollision(uint16_t w, uint16_t h);

    uint16_t get_width()  const { return width; }
    uint16_t get_height() const { return height; }

    bool in_bounds(uint16_t x, uint16_t y) const;
    bool is_occupied(uint16_t x, uint16_t y) const;

    void update(uint16_t x, uint16_t y, bool occ);
    void update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occ);

    void revisar(const WorldPlayers& players, const WorldNpcs& npcs);
};

#endif // WORLD_COLLISION_H