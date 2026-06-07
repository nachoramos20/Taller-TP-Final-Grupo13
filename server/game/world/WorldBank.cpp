#include "WorldBank.h"
#include "WorldPlayers.h"
#include "WorldChat.h"
#include "../Items.h"

#include <sstream>

bool WorldBank::deposit_item(uint16_t client_id, uint8_t inv_slot) {
    PlayerData* p = players.find_mutable(client_id);
    if (!p || p->is_ghost || inv_slot >= PlayerData::INVENTORY_SIZE) return false;

    uint8_t item = p->inventory[inv_slot];
    if (item == 0) return false;

    p->inventory[inv_slot] = 0;
    inventories[p->username].push_back(item);
    chat.push_message(client_id, 0, "Depositaste el objeto en el banco.");
    return true;
}

bool WorldBank::withdraw_item(uint16_t client_id, const std::string& item_name) {
    PlayerData* p = players.find_mutable(client_id);
    if (!p || p->is_ghost) return false;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        chat.push_message(client_id, 0, "Inventario lleno.");
        return false;
    }

    auto& inv = inventories[p->username];
    for (auto it = inv.begin(); it != inv.end(); ++it) {
        if (Items::exists(static_cast<ItemId>(*it))) {
            const auto& def = Items::get(static_cast<ItemId>(*it));
            if (def.name == item_name) {
                p->inventory[free_slot] = *it;
                inv.erase(it);
                chat.push_message(client_id, 0, "Retiraste " + item_name + " del banco.");
                return true;
            }
        }
    }
    chat.push_message(client_id, 0, "No encontraste ese objeto en el banco.");
    return false;
}

bool WorldBank::deposit_gold(uint16_t client_id, uint32_t amount) {
    PlayerData* p = players.find_mutable(client_id);
    if (!p || p->is_ghost || amount == 0 || p->gold < amount) return false;

    p->gold -= amount;
    gold[p->username] += amount;
    chat.push_message(client_id, 0, "Depositaste " + std::to_string(amount) + " de oro en el banco.");
    return true;
}

bool WorldBank::withdraw_gold(uint16_t client_id, uint32_t amount) {
    PlayerData* p = players.find_mutable(client_id);
    if (!p || p->is_ghost || amount == 0) return false;

    auto it = gold.find(p->username);
    if (it == gold.end() || it->second < amount) {
        chat.push_message(client_id, 0, "No tenés suficiente oro en el banco.");
        return false;
    }
    it->second -= amount;
    p->gold += amount;
    chat.push_message(client_id, 0, "Retiraste " + std::to_string(amount) + " de oro del banco.");
    return true;
}

std::string WorldBank::list(uint16_t client_id) const {
    const PlayerData* p = players.find(client_id);
    if (!p) return "";

    std::ostringstream oss;
    oss << "=== Banco de " << p->username << " ===\n";

    auto git = gold.find(p->username);
    oss << "Oro: " << (git != gold.end() ? git->second : 0) << "\n";

    auto iit = inventories.find(p->username);
    if (iit != inventories.end()) {
        for (uint8_t raw : iit->second) {
            if (Items::exists(static_cast<ItemId>(raw)))
                oss << "- " << Items::get(static_cast<ItemId>(raw)).name << "\n";
        }
    }
    return oss.str();
}
