#ifndef WORLD_PLAYERS_H
#define WORLD_PLAYERS_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../PlayerData.h"

class WorldCollision;

class WorldPlayers {
private:
    std::unordered_map<uint16_t, PlayerData> players_map;
    WorldCollision& collision;

    void set_direction_from_delta(PlayerData& player, int dx, int dy);
public:
    explicit WorldPlayers(WorldCollision& c) : collision(c) {}

    void add(const PlayerData& player_data);
    void remove(uint16_t client_id);
    void move(uint16_t client_id, uint16_t new_x, uint16_t new_y);

    const std::unordered_map<uint16_t, PlayerData>& all() const { return players_map; }
    std::unordered_map<uint16_t, PlayerData>& all_mutable() { return players_map; }

    const PlayerData* find(uint16_t client_id) const;
    PlayerData* find_mutable(uint16_t client_id);

    uint16_t find_by_name(const std::string& name) const;
};

#endif
