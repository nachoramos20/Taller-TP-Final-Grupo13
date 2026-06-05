#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>

#include "../../common/protocol/dtos.h"
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
    std::unordered_map<uint16_t, PlayerData> players_map;

    void set_direction_from_delta(PlayerData& player, int dx, int dy);

public:
    World(uint16_t width, uint16_t height);

    // Players
    void add_player(const PlayerData& player_data);
    void remove_player(uint16_t client_id);
    void move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y);

    const std::unordered_map<uint16_t, PlayerData>& get_players() const;
    const PlayerData* find_player(uint16_t client_id) const;
    PlayerData* get_player_mutable(uint16_t client_id);

    // Mundo / colisiones
    void update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied);
    void revisar_colisiones();

    // Snapshot helpers
    std::shared_ptr<std::vector<EntityDTO>> get_entities() const;
};

#endif // WORLD_H
