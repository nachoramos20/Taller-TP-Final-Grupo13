#include "Commands.h"
#include "../Items.h"
#include <sstream>

void ChatCommand::handle_depositar(World& world, const std::string& args) {
    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (first == "oro") {
        uint32_t amount = 0;
        ss >> amount;
        world.bank_deposit_gold(client_id, amount);
    } else {
        PlayerData* p = world.get_player_mutable(client_id);
        if (!p) return;
        std::string item_name = args;
        for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
            if (p->inventory[i] == 0) continue;
            if (Items::exists(static_cast<ItemId>(p->inventory[i]))) {
                const auto& def = Items::get(static_cast<ItemId>(p->inventory[i]));
                if (def.name == item_name) {
                    world.bank_deposit_item(client_id, static_cast<uint8_t>(i));
                    return;
                }
            }
        }
        world.push_message(client_id, 0, "No encontraste ese objeto en tu inventario.");
    }
}

void ChatCommand::handle_retirar(World& world, const std::string& args) {
    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (first == "oro") {
        uint32_t amount = 0;
        ss >> amount;
        world.bank_withdraw_gold(client_id, amount);
    } else {
        world.bank_withdraw_item(client_id, args);
    }
}

void ChatCommand::handle_listar(World& world) {
    std::string listing = world.bank_list(client_id);
    world.push_message(client_id, 0, listing.empty() ? "Banco vacío." : listing);
}
