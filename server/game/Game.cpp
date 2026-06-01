#include "Game.h"

#include <memory>
#include <utility>

Game::Game() : world(100, 100) {}

const std::unordered_map<uint16_t, PlayerData>& Game::get_players() const {
    return players_map;
}

void Game::add_player(const PlayerData& player_data) {
    players_map.emplace(player_data.entity_id, std::move(player_data));
}
void Game::remove_player(uint16_t client_id) {
    players_map.erase(client_id);
}

void Game::move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;

    PlayerData& player = it->second;
    world.move_player(player, std::make_pair(new_x, new_y));
}

SnapshotDTO Game::build_snapshot(uint16_t client_id,
                                 uint32_t tick,
                                 const std::shared_ptr<std::vector<EntityDTO>>& entities) const {
    SnapshotDTO snap{};
    snap.tick = tick;

    auto it = players_map.find(client_id);
    if (it != players_map.end()) {
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
    }

    snap.meditating = 0;

    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; ++i) {
        snap.inventory[i] = 0;
    }

    snap.equipped_wpn  = 0;
    snap.equipped_arm  = 0;
    snap.equipped_helm = 0;
    snap.equipped_shld = 0;

    snap.entities = entities;

    return snap;
}

void Game::revisar_colisiones() {
    for (auto& [id, player] : players_map) {
        std::pair<uint16_t, uint16_t> pos = {player.pos_x, player.pos_y};
        world.update_occupied(pos, true);
    }
}

std::shared_ptr<std::vector<EntityDTO>> Game::get_entities() const {
    std::shared_ptr<std::vector<EntityDTO>> entities = std::make_shared<std::vector<EntityDTO>>();
    for (const auto& [id, player] : this->players_map) {
        EntityDTO entity{};
        entity.entity_id   = player.entity_id;
        entity.entity_type = 0;
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