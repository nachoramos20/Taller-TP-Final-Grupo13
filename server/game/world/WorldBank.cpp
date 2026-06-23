#include "WorldBank.h"

#include <sstream>

#include "../Items.h"

#include "WorldChat.h"
#include "WorldPlayers.h"

static int find_free_bank_slot(const PlayerData& p) {
    for (int i = 0; i < PlayerData::BANK_SIZE; ++i)
        if (p.bank[i] == 0)
            return i;
    return -1;
}

bool WorldBank::deposit_item(uint16_t client_id, uint8_t inv_slot) {
    PlayerData* p = players.find_mutable(client_id);
    if (!p || p->is_ghost || inv_slot >= PlayerData::INVENTORY_SIZE)
        return false;

    uint8_t item = p->inventory[inv_slot];
    if (item == 0)
        return false;

    int free_slot = find_free_bank_slot(*p);
    if (free_slot == -1) {
        chat.push_message(client_id, 0, "El banco está lleno.");
        return false;
    }

    p->inventory[inv_slot] = 0;
    p->bank[free_slot] = item;
    chat.push_message(client_id, 0, "Depositaste el objeto en el banco.");
    return true;
}

bool WorldBank::withdraw_item(uint16_t client_id, const std::string& item_name) {
    PlayerData* p = players.find_mutable(client_id);
    if (!p || p->is_ghost)
        return false;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) {
            free_slot = i;
            break;
        }
    if (free_slot == -1) {
        chat.push_message(client_id, 0, "Inventario lleno.");
        return false;
    }

    for (int i = 0; i < PlayerData::BANK_SIZE; ++i) {
        uint8_t item_id = p->bank[i];
        if (item_id == 0)
            continue;
        if (Items::exists(static_cast<ItemId>(item_id))) {
            const auto& def = Items::get(static_cast<ItemId>(item_id));
            if (Items::name_equals_ci(def.name, item_name)) {
                p->inventory[free_slot] = item_id;
                p->bank[i] = 0;
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
    if (!p || p->is_ghost || amount == 0 || p->gold < amount)
        return false;

    p->gold -= amount;
    p->bank_gold += amount;
    chat.push_message(client_id, 0,
                      "Depositaste " + std::to_string(amount) + " de oro en el banco.");
    return true;
}

bool WorldBank::withdraw_gold(uint16_t client_id, uint32_t amount) {
    PlayerData* p = players.find_mutable(client_id);
    if (!p || p->is_ghost || amount == 0)
        return false;

    if (p->bank_gold < amount) {
        chat.push_message(client_id, 0, "No tenés suficiente oro en el banco.");
        return false;
    }
    p->bank_gold -= amount;
    p->gold += amount;
    chat.push_message(client_id, 0, "Retiraste " + std::to_string(amount) + " de oro del banco.");
    return true;
}

std::string WorldBank::list(uint16_t client_id) const {
    const PlayerData* p = players.find(client_id);
    if (!p)
        return "";

    std::ostringstream oss;
    oss << "=== Banco de " << p->username << " ===\n";
    oss << "Oro: " << p->bank_gold << "\n";

    for (int i = 0; i < PlayerData::BANK_SIZE; ++i) {
        uint8_t item_id = p->bank[i];
        if (item_id == 0)
            continue;
        if (Items::exists(static_cast<ItemId>(item_id)))
            oss << "- " << Items::get(static_cast<ItemId>(item_id)).name << "\n";
    }
    return oss.str();
}