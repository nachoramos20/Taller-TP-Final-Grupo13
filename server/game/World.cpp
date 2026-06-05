#include "World.h"

#include <cstdlib>
#include <utility>

#include "../../common/protocol/protocol.h"

World::World(uint16_t width, uint16_t height) : width(width), height(height) {
    for (uint16_t x = 0; x < width; ++x) {
        for (uint16_t y = 0; y < height; ++y) {
            occupied_positions[std::make_pair(x, y)] = false;
        }
    }
}

void World::add_player(const PlayerData& player_data) {
    players_map.emplace(player_data.entity_id, player_data);
    update_occupied({player_data.pos_x, player_data.pos_y}, true);
}

void World::remove_player(uint16_t client_id) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;
    update_occupied({it->second.pos_x, it->second.pos_y}, false);
    players_map.erase(it);
}

void World::set_direction_from_delta(PlayerData& player, int dx, int dy) {
    if (dx == 1)       player.direction = static_cast<uint8_t>(MoveDirection::EAST);
    else if (dx == -1) player.direction = static_cast<uint8_t>(MoveDirection::WEST);
    else if (dy == 1)  player.direction = static_cast<uint8_t>(MoveDirection::SOUTH);
    else if (dy == -1) player.direction = static_cast<uint8_t>(MoveDirection::NORTH);
}

void World::move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;
    PlayerData& player = it->second;

    if (new_x >= width || new_y >= height) return;

    const int dx = static_cast<int>(new_x) - static_cast<int>(player.pos_x);
    const int dy = static_cast<int>(new_y) - static_cast<int>(player.pos_y);
    if (std::abs(dx) + std::abs(dy) != 1) return;

    set_direction_from_delta(player, dx, dy);

    const std::pair<uint16_t, uint16_t> new_pos{new_x, new_y};
    if (occupied_positions[new_pos]) return;

    occupied_positions[{player.pos_x, player.pos_y}] = false;
    occupied_positions[new_pos] = true;

    player.pos_x = new_x;
    player.pos_y = new_y;
}

const std::unordered_map<uint16_t, PlayerData>& World::get_players() const {
    return players_map;
}

const PlayerData* World::find_player(uint16_t client_id) const {
    auto it = players_map.find(client_id);
    return it == players_map.end() ? nullptr : &it->second;
}

void World::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied) {
    if (pos.first >= width || pos.second >= height) return;
    occupied_positions[pos] = occupied;
}

void World::revisar_colisiones() {
    for (auto& [id, player] : players_map) {
        update_occupied({player.pos_x, player.pos_y}, true);
    }
}

std::shared_ptr<std::vector<EntityDTO>> World::get_entities() const {
    auto entities = std::make_shared<std::vector<EntityDTO>>();
    entities->reserve(players_map.size());
    for (const auto& [id, player] : players_map) {
        EntityDTO entity{};
        entity.entity_id   = player.entity_id;
        entity.entity_type = 0; // PLAYER
        entity.username    = player.username;
        entity.pos_x       = player.pos_x;
        entity.pos_y       = player.pos_y;
        entity.direction   = player.direction;
        entity.sprite_id   = static_cast<uint8_t>(player.race + 1);
        entity.is_ghost    = player.is_ghost ? 1 : 0;
        entity.hp_pct      = static_cast<uint8_t>(
            player.max_hp > 0 ? (player.hp * 100) / player.max_hp : 0);
        entities->push_back(entity);
    }
    return entities;
}

PlayerData* World::get_player_mutable(uint16_t client_id) {
    auto it = players_map.find(client_id);
    return it == players_map.end() ? nullptr : &it->second;
}
