#pragma once

#include <cstdint>
#include <unordered_map>
#include <cmath>
#include "../../common/protocol/dtos.h"
#include "../../common/protocol/protocol.h"

static constexpr int SERVER_MAP_SIZE = 100;

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

class World {
public:
    void add_player(uint16_t client_id) {
        PlayerData p;
        p.entity_id = client_id;
        p.pos_x     = SERVER_MAP_SIZE / 2;
        p.pos_y     = SERVER_MAP_SIZE / 2;
        p.direction = 1;
        p.hp        = 100;
        p.max_hp    = 100;
        p.mp        = 100;
        p.max_mp    = 100;
        p.exp       = 0;
        p.level     = 1;
        p.gold      = 0;
        p.is_ghost  = false;
        _players[client_id] = p;
    }

    void remove_player(uint16_t client_id) {
        _players.erase(client_id);
    }

    void move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y) {
        auto it = _players.find(client_id);
        if (it == _players.end()) return;

        if (new_x >= SERVER_MAP_SIZE || new_y >= SERVER_MAP_SIZE) return;

        PlayerData& p = it->second;
        int dx = static_cast<int>(new_x) - static_cast<int>(p.pos_x);
        int dy = static_cast<int>(new_y) - static_cast<int>(p.pos_y);
        if (std::abs(dx) + std::abs(dy) != 1) return;

        if      (dy == -1) p.direction = 0;
        else if (dy == +1) p.direction = 1;
        else if (dx == +1) p.direction = 2;
        else if (dx == -1) p.direction = 3;

        p.pos_x = new_x;
        p.pos_y = new_y;
    }

    SnapshotDTO build_snapshot(uint16_t client_id, uint32_t tick) const {
        SnapshotDTO snap;
        snap.tick = tick;

        auto it = _players.find(client_id);
        if (it != _players.end()) {
            const PlayerData& p = it->second;
            snap.self_entity_id = p.entity_id;
            snap.hp             = p.hp;
            snap.max_hp         = p.max_hp;
            snap.mp             = p.mp;
            snap.max_mp         = p.max_mp;
            snap.exp            = p.exp;
            snap.level          = p.level;
            snap.gold           = p.gold;
            snap.is_ghost       = p.is_ghost ? 1 : 0;
            snap.meditating     = 0;
        }

        snap.equipped_wpn  = 0;
        snap.equipped_arm  = 0;
        snap.equipped_helm = 0;
        snap.equipped_shld = 0;

        snap.entities.reserve(_players.size());
        for (const auto& [id, p] : _players) {
            EntityDTO e;
            e.entity_id   = p.entity_id;
            e.entity_type = 0;
            e.pos_x       = p.pos_x;
            e.pos_y       = p.pos_y;
            e.direction   = p.direction;
            e.sprite_id   = 0;
            e.is_ghost    = p.is_ghost ? 1 : 0;
            e.hp_pct      = static_cast<uint8_t>(
                p.max_hp > 0 ? (p.hp * 100) / p.max_hp : 0);
            snap.entities.push_back(e);
        }

        return snap;
    }

    bool has_player(uint16_t client_id) const {
        return _players.count(client_id) > 0;
    }

    const std::unordered_map<uint16_t, PlayerData>& players() const {
        return _players;
    }

private:
    std::unordered_map<uint16_t, PlayerData> _players;
};