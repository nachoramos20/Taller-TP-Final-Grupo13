#include "Game.h"
#include <utility>

Game::Game() : world(100, 100) {}

const std::unordered_map<uint16_t, PlayerData>& Game::get_players() const {
    return players_map;
}

void Game::add_player(uint16_t client_id, const std::string& username) {
    PlayerData pd{};
    pd.entity_id = client_id;
    pd.username = username;
    pd.pos_x = 50;
    pd.pos_y = 50;
    pd.direction = 0;
    pd.hp = pd.max_hp = 100;
    pd.mp = pd.max_mp = 100;
    pd.exp = 0;
    pd.level = 1;
    pd.gold = 0;
    pd.is_ghost = false;

    players_map.emplace(client_id, std::move(pd));
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

SnapshotDTO Game::build_snapshot(uint16_t client_id, uint32_t tick) const {
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

    snap.entities.reserve(players_map.size());
    for (const auto& [id, p] : players_map) {
        EntityDTO entity;
        entity.entity_id   = p.entity_id;
        entity.entity_type = 0;
        entity.pos_x       = p.pos_x;
        entity.pos_y       = p.pos_y;
        entity.direction   = p.direction;
        entity.sprite_id   = 0;
        entity.is_ghost    = p.is_ghost ? 1 : 0;
        entity.hp_pct      = static_cast<uint8_t>(
            p.max_hp > 0 ? (p.hp * 100) / p.max_hp : 0);
        snap.entities.push_back(std::move(entity));
    }

    return snap;
}

void Game::revisar_colisiones() {
    for (auto& [id, player] : players_map) {
        std::pair<uint16_t, uint16_t> pos = {player.pos_x, player.pos_y};
        world.update_occupied(pos, true);
    }
}