#include "WorldPlayers.h"
#include "WorldCollision.h"
#include "../../../common/protocol/protocol.h"

#include <cstdlib>

void WorldPlayers::add(const PlayerData& player_data) {
    players_map.emplace(player_data.entity_id, player_data);
    collision.update(player_data.pos_x, player_data.pos_y, true);
}

void WorldPlayers::remove(uint16_t client_id) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;
    save_queue.push(it->second);
    collision.update(it->second.pos_x, it->second.pos_y, false);
    players_map.erase(it);
}

void WorldPlayers::set_direction_from_delta(PlayerData& player, int dx, int dy) {
    if (dx == 1)       player.direction = static_cast<uint8_t>(MoveDirection::EAST);
    else if (dx == -1) player.direction = static_cast<uint8_t>(MoveDirection::WEST);
    else if (dy == 1)  player.direction = static_cast<uint8_t>(MoveDirection::SOUTH);
    else if (dy == -1) player.direction = static_cast<uint8_t>(MoveDirection::NORTH);
}

void WorldPlayers::tp(uint16_t client_id, uint16_t new_x, uint16_t new_y) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;
    PlayerData& player = it->second;

    if (!collision.in_bounds(new_x, new_y)) return;

    collision.update(player.pos_x, player.pos_y, false);
    collision.update(new_x, new_y, true);

    player.pos_x = new_x;
    player.pos_y = new_y;
    player.meditating = false;
}

void WorldPlayers::move(uint16_t client_id, uint16_t new_x, uint16_t new_y) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;
    PlayerData& player = it->second;

    if (!collision.in_bounds(new_x, new_y)) return;

    const int dx = static_cast<int>(new_x) - static_cast<int>(player.pos_x);
    const int dy = static_cast<int>(new_y) - static_cast<int>(player.pos_y);
    if (std::abs(dx) + std::abs(dy) != 1) return;

    set_direction_from_delta(player, dx, dy);

    if (collision.is_occupied(new_x, new_y)) return;

    collision.update(player.pos_x, player.pos_y, false);
    collision.update(new_x, new_y, true);

    player.pos_x = new_x;
    player.pos_y = new_y;
    player.meditating = false;
}

const PlayerData* WorldPlayers::find(uint16_t client_id) const {
    auto it = players_map.find(client_id);
    return it == players_map.end() ? nullptr : &it->second;
}

PlayerData* WorldPlayers::find_mutable(uint16_t client_id) {
    auto it = players_map.find(client_id);
    return it == players_map.end() ? nullptr : &it->second;
}

bool WorldPlayers::kick_by_username(const std::string& name) {
    for (auto it = players_map.begin(); it != players_map.end(); ++it) {
        if (std::string(it->second.username) == name) {
            save_queue.push(it->second);
            collision.update(it->second.pos_x, it->second.pos_y, false);
            players_map.erase(it);
            return true;
        }
    }
    return false;
}

uint16_t WorldPlayers::find_by_name(const std::string& name) const {
    for (const auto& [id, p] : players_map)
        if (std::string(p.username) == name) return id;
    return 0;
}
