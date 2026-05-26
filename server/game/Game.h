#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include "../../common/protocol/dtos.h"
#include "World.h"
#include "PlayerData.h"

class Game {
    private:
        World world;
        std::unordered_map<uint16_t, PlayerData> players_map;

    public:
        Game();
        const std::unordered_map<uint16_t, PlayerData>& get_players() const;
        void add_player(uint16_t client_id, const std::string& username);
        void remove_player(uint16_t client_id);
        void move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y);
        SnapshotDTO build_snapshot(uint16_t client_id, uint32_t tick) const;
        void revisar_colisiones();

};

#endif // GAME_H