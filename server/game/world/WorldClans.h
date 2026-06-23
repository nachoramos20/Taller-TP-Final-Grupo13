#ifndef WORLD_CLANS_H
#define WORLD_CLANS_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "../entities/Clan.h"

class WorldPlayers;
class WorldChat;

// Clanes, su membresía y los pedidos de ingreso pendientes.
class WorldClans {
private:
    std::unordered_map<std::string, Clan> clans_by_name;
    std::unordered_map<uint16_t, std::string> player_clan;

    WorldPlayers& players;
    WorldChat& chat;

public:
    WorldClans(WorldPlayers& p, WorldChat& c): players(p), chat(c) {}

    bool found(uint16_t founder_id, const std::string& clan_name);
    bool join_request(uint16_t player_id, const std::string& clan_name);
    std::string review(uint16_t founder_id) const;
    bool accept(uint16_t founder_id, const std::string& nick);
    bool reject(uint16_t founder_id, const std::string& nick);
    bool ban(uint16_t founder_id, const std::string& nick);
    bool kick(uint16_t founder_id, const std::string& nick);
    bool leave(uint16_t player_id);
    bool same_clan(uint16_t a, uint16_t b) const;
    void restore_membership(uint16_t player_id, const std::string& clan_name, bool is_founder);
    void notify_login(uint16_t player_id, bool online);
    void notify_attack(uint16_t attacked_id);
};

#endif
