#ifndef PERSISTENCE_MONITOR_H
#define PERSISTENCE_MONITOR_H

#include "entities/PlayerData.h"
#include "../../common/queue.h"

#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>

// Persiste y recupera PlayerData en dos archivos binarios: un índice
// (username → offset) y los datos completos. Login/registro/guardado son
// thread-safe (mtx) porque distintos ClientHandler corren en paralelo.
class PersistenceMonitor {
public:
    PersistenceMonitor(Queue<PlayerData>& save_queue);

    bool login(const std::string& target_username, PlayerData& player_data, uint16_t entity_id);
    bool register_user(const std::string& new_username, uint8_t race, uint8_t cls, PlayerData& player_data, uint16_t entity_id);
    void save_player(const PlayerData& player_data_to_save);

    PersistenceMonitor(const PersistenceMonitor&) = delete;
    PersistenceMonitor& operator=(const PersistenceMonitor&) = delete;

private:
    Queue<PlayerData>& save_queue;
    std::unordered_map<std::string, uint64_t> player_offsets_map;
    std::mutex mtx;
    
    static constexpr const char* PLAYERS_DATA_FILENAME = "players_data.bin";
    static constexpr const char* PLAYERS_INDEX_FILENAME = "players_index.bin";

    struct IndexEntry {
        char username[PlayerData::USERNAME_MAX_LENGTH + 1];
        uint64_t offset;
    };
    static PlayerData make_initial_player(const std::string& username, uint8_t race, uint8_t cls);

};

#endif // PERSISTENCE_MONITOR_H