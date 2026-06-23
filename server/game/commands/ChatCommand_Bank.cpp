#include "Commands.h"
#include "../Items.h"
#include <sstream>
#include <unordered_map>

// ─── helper interno ────────────────────────────────────────────────────────
static bool check_banker(World& world, uint16_t client_id) {
    if (!world.player_near_service_npc(client_id, NpcId::BANKER)) {
        world.push_message(client_id, 0,
            "Debes estar cerca del Banquero para usar el banco.\n"
            "Acércate y haz click en él primero.");
        return false;
    }
    return true;
}

void ChatCommand::handle_depositar(World& world, const std::string& args) {
    if (!check_banker(world, client_id)) return;

    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (Items::name_equals_ci(first, "oro")) {
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
                if (Items::name_equals_ci(def.name, item_name)) {
                    world.bank_deposit_item(client_id, static_cast<uint8_t>(i));
                    return;
                }
            }
        }
        world.push_message(client_id, 0, "No encontraste ese objeto en tu inventario.");
    }
}

void ChatCommand::handle_retirar(World& world, const std::string& args) {
    if (!check_banker(world, client_id)) return;

    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (Items::name_equals_ci(first, "oro")) {
        uint32_t amount = 0;
        ss >> amount;
        world.bank_withdraw_gold(client_id, amount);
    } else {
        world.bank_withdraw_item(client_id, args);
    }
}

void ChatCommand::handle_listar(World& world) {
    // /listar funciona tanto para banco (si hay banquero cerca) como para comerciante
    if (world.player_near_service_npc(client_id, NpcId::BANKER)) {
        std::string listing = world.bank_list(client_id);
        world.push_message(client_id, 0, listing.empty() ? "Banco vacío." : listing);
        return;
    }
    if (world.player_near_service_npc(client_id, NpcId::MERCHANT)) {
        handle_listar_comerciante(world);
        return;
    }
    world.push_message(client_id, 0,
        "Debes estar cerca del Banquero o Comerciante para usar este comando.");
}

// Catálogo del comerciante por zona (Factory/tabla en vez de switch):
// zone_id 0 = Ciudad, 1 = Pueblo; cualquier otro valor cae al catálogo
// básico. Declarada en Commands.h porque también la usa
// ChatCommand::handle_comprar en ChatCommands.cpp.
std::vector<ItemId> merchant_catalog_for_zone(uint8_t zone_id) {
    static const std::unordered_map<uint8_t, std::vector<ItemId>> catalogs = {
        {0, { // Ciudad
            ItemId::SWORD, ItemId::COMPOUND_BOW, ItemId::GEMMED_STAFF,
            ItemId::PLATE_ARMOR, ItemId::IRON_HELMET, ItemId::IRON_SHIELD,
            ItemId::HEALTH_POTION, ItemId::MANA_POTION,
        }},
        {1, { // Pueblo
            ItemId::SWORD, ItemId::SIMPLE_BOW, ItemId::ELVEN_FLUTE,
            ItemId::LEATHER_ARMOR, ItemId::HEALTH_POTION,
        }},
    };
    static const std::vector<ItemId> basic_catalog = {
        ItemId::SWORD, ItemId::LEATHER_ARMOR, ItemId::HEALTH_POTION,
    };

    auto it = catalogs.find(zone_id);
    return it != catalogs.end() ? it->second : basic_catalog;
}

void ChatCommand::handle_listar_comerciante(World& world) {
    uint8_t zone = world.get_nearby_merchant_zone(client_id);
    std::vector<ItemId> shop_items = merchant_catalog_for_zone(zone);

    std::string zone_name = (zone == 0) ? "Ciudad" : (zone == 1) ? "Pueblo" : "";
    std::string header = zone_name.empty()
        ? "=== Tienda del Comerciante ===\n"
        : "=== Tienda del Comerciante (" + zone_name + ") ===\n";

    std::string msg = header;
    for (ItemId iid : shop_items) {
        const ItemDef& def = Items::get(iid);
        uint32_t price = (static_cast<uint32_t>(def.min_value) + def.max_value) * 8 + 50;
        msg += def.name + " - " + std::to_string(price) + " oro\n";
    }
    world.push_message(client_id, 0, msg);
}
