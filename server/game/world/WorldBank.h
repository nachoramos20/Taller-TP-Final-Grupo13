#ifndef WORLD_BANK_H
#define WORLD_BANK_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class WorldPlayers;
class WorldChat;

class WorldBank {
private:
    std::unordered_map<std::string, std::vector<uint8_t>> inventories;
    std::unordered_map<std::string, uint32_t>             gold;

    WorldPlayers& players;
    WorldChat&    chat;
public:
    WorldBank(WorldPlayers& p, WorldChat& c) : players(p), chat(c) {}

    bool deposit_item(uint16_t client_id, uint8_t inv_slot);
    bool withdraw_item(uint16_t client_id, const std::string& item_name);
    bool deposit_gold(uint16_t client_id, uint32_t amount);
    bool withdraw_gold(uint16_t client_id, uint32_t amount);
    std::string list(uint16_t client_id) const;
};

#endif
