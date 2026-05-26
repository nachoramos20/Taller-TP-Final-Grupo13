#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <unordered_map>
#include <functional>
#include "PlayerData.h"
namespace std {
    template <>
    struct hash<std::pair<uint16_t, uint16_t>> {
        size_t operator()(const std::pair<uint16_t, uint16_t>& p) const {
            return hash<uint16_t>()(p.first) ^ (hash<uint16_t>()(p.second) << 1);
        }
    };
}

class World {
    private:
        uint16_t width;
        uint16_t height;
        std::unordered_map<std::pair<uint16_t, uint16_t>, bool> occupied_positions;

    public:
        World(uint16_t width, uint16_t height);
        void move_player(PlayerData& player, const std::pair<uint16_t, uint16_t>& new_pos);
        void update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied);
};

#endif // WORLD_H