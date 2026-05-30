#include "PersistenceMonitor.h"

PlayerData PersistenceMonitor::make_initial_player(const std::string& username) {
    PlayerData data{};
    data.username = username;
    data.entity_id = 0;
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

bool PersistenceMonitor::login_or_register(const std::string& username, PlayerData& out_data, uint16_t entity_id) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = mock_db.find(username);
    if (it != mock_db.end()) {
        out_data = it->second;
        out_data.entity_id = entity_id;
        return true;
    }

    PlayerData new_player = make_initial_player(username);
    new_player.entity_id = entity_id;
    mock_db[username] = new_player;
    out_data = new_player;
    return true;
}

void PersistenceMonitor::save_player(const PlayerData& data) {
    std::lock_guard<std::mutex> lock(mtx);
    mock_db[data.username] = data;
}