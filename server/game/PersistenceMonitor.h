#ifndef PERSISTENCE_MONITOR_H
#define PERSISTENCE_MONITOR_H

#include "PlayerData.h"

#include <mutex>
#include <string>
#include <unordered_map>

class PersistenceMonitor {
public:
    PersistenceMonitor() = default;

    bool login_or_register(const std::string& username, PlayerData& out_data, uint16_t entity_id);
    void save_player(const PlayerData& data);

    PersistenceMonitor(const PersistenceMonitor&) = delete;
    PersistenceMonitor& operator=(const PersistenceMonitor&) = delete;

private:
    static PlayerData make_initial_player(const std::string& username);

    std::unordered_map<std::string, PlayerData> mock_db; // por el momento usamos esto para que funcione, despues se usa el archivo de persistencia
    std::mutex mtx;
};

#endif // PERSISTENCE_MONITOR_H