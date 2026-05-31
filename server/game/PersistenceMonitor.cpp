#include "PersistenceMonitor.h"

PlayerData PersistenceMonitor::make_initial_player(const std::string& username, uint8_t race, uint8_t cls) {
    PlayerData data{};
    data.username = username;
    data.entity_id = 0;
    data.race = race;
    data.cls = cls;
    data.pos_x = 50;
    data.pos_y = 50;
    data.direction = 0;
    data.hp = 100;
    data.max_hp = 100;
    data.mp = 100;
    data.max_mp = 100;
    data.exp = 0;
    data.level = 1;
    data.gold = 0;
    data.is_ghost = false;
    return data;
}

bool PersistenceMonitor::login(const std::string& username, PlayerData& player_data, uint16_t entity_id) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = mock_db.find(username);
    if (it != mock_db.end()) {
        player_data = it->second;
        player_data.entity_id = entity_id;
        return true;
    }

    return false;
}

bool PersistenceMonitor::register_user(const std::string& username, uint8_t race, uint8_t cls, PlayerData& player_data, uint16_t entity_id) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = mock_db.find(username);
    if (it != mock_db.end()) {
        return false;
    }

    PlayerData new_player = make_initial_player(username, race, cls);
    new_player.entity_id = entity_id;
    mock_db[username] = new_player;
    player_data = new_player;
    return true;
}

void PersistenceMonitor::save_player(const PlayerData& data) {
    std::lock_guard<std::mutex> lock(mtx);
    mock_db[data.username] = data;
}