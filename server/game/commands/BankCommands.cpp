#include "BankCommands.h"

#include <sstream>

#include "../Items.h"
#include "../entities/PlayerData.h"

namespace {
bool check_banker(World& world, uint16_t client_id) {
    if (!world.player_near_service_npc(client_id, NpcId::BANKER)) {
        world.push_message(client_id, 0,
                           "Debes estar cerca del Banquero para usar el banco.\n"
                           "Acércate y haz click en él primero.");
        return false;
    }
    return true;
}
}  // namespace

BankCommands::BankCommands(uint16_t client_id): client_id_(client_id) {}

void BankCommands::deposit(World& world, const std::string& args) {
    if (!check_banker(world, client_id_))
        return;

    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (Items::name_equals_ci(first, "oro")) {
        uint32_t amount = 0;
        ss >> amount;
        world.bank_deposit_gold(client_id_, amount);
    } else {
        PlayerData* p = world.get_player_mutable(client_id_);
        if (!p)
            return;
        std::string item_name = args;
        for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
            if (p->inventory[i] == 0)
                continue;
            if (Items::exists(static_cast<ItemId>(p->inventory[i]))) {
                const auto& def = Items::get(static_cast<ItemId>(p->inventory[i]));
                if (Items::name_equals_ci(def.name, item_name)) {
                    world.bank_deposit_item(client_id_, static_cast<uint8_t>(i));
                    return;
                }
            }
        }
        world.push_message(client_id_, 0, "No encontraste ese objeto en tu inventario.");
    }
}

void BankCommands::withdraw(World& world, const std::string& args) {
    if (!check_banker(world, client_id_))
        return;

    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (Items::name_equals_ci(first, "oro")) {
        uint32_t amount = 0;
        ss >> amount;
        world.bank_withdraw_gold(client_id_, amount);
    } else {
        world.bank_withdraw_item(client_id_, args);
    }
}

bool BankCommands::try_list_account(World& world) {
    if (!world.player_near_service_npc(client_id_, NpcId::BANKER))
        return false;
    std::string listing = world.bank_list(client_id_);
    world.push_message(client_id_, 0, listing.empty() ? "Banco vacío." : listing);
    return true;
}
