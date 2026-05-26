#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <unordered_map>
#include "../../common/protocol/dtos.h"


struct PlayerData {
    uint16_t entity_id;
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t  direction;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t mp;
    uint16_t max_mp;
    uint32_t exp;
    uint8_t  level;
    uint32_t gold;
    bool     is_ghost;

};

class Game {
    private:
        std::unordered_map<uint16_t, PlayerData> players_map;

    public:
        const std::unordered_map<uint16_t, PlayerData>& get_players() const;
        void add_player(uint16_t client_id);
        void remove_player(uint16_t client_id);
        void move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y);
        SnapshotDTO build_snapshot(uint16_t client_id, uint32_t tick) const;

};

#endif // GAME_H