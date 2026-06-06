#include "PersistenceMonitor.h"

#include <fstream>

PersistenceMonitor::PersistenceMonitor(Queue<PlayerData>& save_queue): save_queue(save_queue) {
    std::ifstream index_file(PLAYERS_INDEX_FILENAME, std::ios::in | std::ios::binary);
    if (!index_file.is_open()) {
        return;
    }

    IndexEntry entry{};
    while (index_file.read(reinterpret_cast<char*>(&entry), sizeof(entry))) {
        player_offsets_map[std::string(entry.username)] = entry.offset;
    }

    index_file.close();
}

PlayerData PersistenceMonitor::
make_initial_player(const std::string& username, uint8_t race, uint8_t cls) {
    PlayerData data{};
    PlayerData::copy_username(data.username, username);
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

bool PersistenceMonitor::login(const std::string& target_username, PlayerData& player_data, uint16_t entity_id) {
    std::lock_guard<std::mutex> lock(mtx);

    if (target_username.size() > PlayerData::USERNAME_MAX_LENGTH) {
        return false;
    }

    auto it = player_offsets_map.find(target_username);
    if (it == player_offsets_map.end()) {
        return false;
    }

    std::ifstream data_file(PLAYERS_DATA_FILENAME, std::ios::in | std::ios::binary);
    if (!data_file.is_open()) {
        return false;
    }

    data_file.seekg(static_cast<std::streamoff>(it->second));
    if (!data_file.good()) {
        return false;
    }

    data_file.read(reinterpret_cast<char*>(&player_data), sizeof(PlayerData));
    if (!data_file) {
        return false;
    }

    player_data.entity_id = entity_id;
    return true;
}

bool PersistenceMonitor::register_user(const std::string& new_username, uint8_t race, uint8_t cls, PlayerData& player_data, uint16_t entity_id) {
    std::lock_guard<std::mutex> lock(mtx);

    if (new_username.size() > PlayerData::USERNAME_MAX_LENGTH) {
        return false;
    }

    if (player_offsets_map.find(new_username) != player_offsets_map.end()) {
        return false;
    }

    PlayerData new_player = make_initial_player(new_username, race, cls);
    new_player.entity_id = entity_id;
    player_data = new_player;

    std::ofstream data_file(PLAYERS_DATA_FILENAME, std::ios::app | std::ios::binary);
    std::ofstream index_file(PLAYERS_INDEX_FILENAME, std::ios::app | std::ios::binary);
    if (!data_file.is_open() || !index_file.is_open()) {
        return false;
    }

    const uint64_t new_offset = static_cast<uint64_t>(data_file.tellp());
    data_file.write(reinterpret_cast<const char*>(&player_data), sizeof(PlayerData));
    if (!data_file) {
        return false;
    }

    IndexEntry index_entry{};
    PlayerData::copy_username(index_entry.username, new_username);
    index_entry.offset = new_offset;
    index_file.write(reinterpret_cast<const char*>(&index_entry), sizeof(IndexEntry));
    if (!index_file) {
        return false;
    }

    player_offsets_map[new_username] = new_offset;

    return true;
}

void PersistenceMonitor::save_player(const PlayerData& player_data_to_save) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = player_offsets_map.find(std::string(player_data_to_save.username));
    if (it == player_offsets_map.end()) {
        return;
    }

    std::fstream data_file(PLAYERS_DATA_FILENAME, std::ios::in | std::ios::out | std::ios::binary);
    if (!data_file.is_open()) {
        return;
    }

    data_file.seekp(static_cast<std::streamoff>(it->second));
    if (!data_file.good()) {
        return;
    }

    data_file.write(reinterpret_cast<const char*>(&player_data_to_save), sizeof(PlayerData));
}