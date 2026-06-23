#include "MerchantCommands.h"

#include <unordered_map>

#include "../Items.h"
#include "../entities/PlayerData.h"

// Catálogo del comerciante por zona (Factory/tabla en vez de switch, ver
// Tarea 1): zone_id 0 = Ciudad, 1 = Pueblo; cualquier otro valor cae al
// catálogo básico.
std::vector<ItemId> merchant_catalog_for_zone(uint8_t zone_id) {
    static const std::unordered_map<uint8_t, std::vector<ItemId>> catalogs = {
            {0,
             {
                     // Ciudad
                     ItemId::SWORD,
                     ItemId::COMPOUND_BOW,
                     ItemId::GEMMED_STAFF,
                     ItemId::PLATE_ARMOR,
                     ItemId::IRON_HELMET,
                     ItemId::IRON_SHIELD,
                     ItemId::HEALTH_POTION,
                     ItemId::MANA_POTION,
             }},
            {1,
             {
                     // Pueblo
                     ItemId::SWORD,
                     ItemId::SIMPLE_BOW,
                     ItemId::ELVEN_FLUTE,
                     ItemId::LEATHER_ARMOR,
                     ItemId::HEALTH_POTION,
             }},
    };
    static const std::vector<ItemId> basic_catalog = {
            ItemId::SWORD,
            ItemId::LEATHER_ARMOR,
            ItemId::HEALTH_POTION,
    };

    auto it = catalogs.find(zone_id);
    return it != catalogs.end() ? it->second : basic_catalog;
}

MerchantCommands::MerchantCommands(uint16_t client_id): client_id_(client_id) {}

void MerchantCommands::buy(World& world, const std::string& item_name) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p || p->is_ghost)
        return;

    if (!world.player_near_service_npc(client_id_, NpcId::MERCHANT)) {
        world.push_message(client_id_, 0, "Debes estar cerca del Comerciante para comprar.");
        return;
    }

    uint8_t zone = world.get_nearby_merchant_zone(client_id_);
    std::vector<ItemId> shop_items = merchant_catalog_for_zone(zone);

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) {
            free_slot = i;
            break;
        }
    if (free_slot == -1) {
        world.push_message(client_id_, 0, "Inventario lleno.");
        return;
    }

    for (ItemId iid: shop_items) {
        const ItemDef& def = Items::get(iid);
        if (Items::name_equals_ci(def.name, item_name)) {
            uint32_t price = (static_cast<uint32_t>(def.min_value) + def.max_value) * 8 + 50;
            if (p->gold < price) {
                world.push_message(
                        client_id_, 0,
                        "Oro insuficiente. Necesitas " + std::to_string(price) + " de oro.");
                return;
            }
            p->gold -= price;
            p->inventory[free_slot] = static_cast<uint8_t>(iid);
            world.push_message(
                    client_id_, 0,
                    "Compraste " + item_name + " por " + std::to_string(price) + " de oro.");
            return;
        }
    }
    world.push_message(client_id_, 0, "El comerciante no tiene ese articulo. Usa /listar.");
}

void MerchantCommands::sell(World& world, const std::string& item_name) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p || p->is_ghost)
        return;

    if (!world.player_near_service_npc(client_id_, NpcId::MERCHANT)) {
        world.push_message(client_id_, 0,
                           "Debes estar cerca del Comerciante para vender.\n"
                           "Acércate y haz click en él primero.");
        return;
    }

    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0)
            continue;
        if (!Items::exists(static_cast<ItemId>(p->inventory[i])))
            continue;
        const ItemDef& def = Items::get(static_cast<ItemId>(p->inventory[i]));
        if (Items::name_equals_ci(def.name, item_name)) {
            uint32_t sell_price = (def.max_value * 10 + 10) / 2;
            p->gold += sell_price;
            p->inventory[i] = 0;

            if (p->equipped_weapon == static_cast<uint8_t>(i))
                p->equipped_weapon = 0xFF;
            if (p->equipped_armor == static_cast<uint8_t>(i))
                p->equipped_armor = 0xFF;
            if (p->equipped_helmet == static_cast<uint8_t>(i))
                p->equipped_helmet = 0xFF;
            if (p->equipped_shield == static_cast<uint8_t>(i))
                p->equipped_shield = 0xFF;

            world.push_message(
                    client_id_, 0,
                    "Vendiste " + item_name + " por " + std::to_string(sell_price) + " de oro.");
            return;
        }
    }
    world.push_message(client_id_, 0, "No tienes ese objeto para vender.");
}

void MerchantCommands::list_catalog(World& world) {
    uint8_t zone = world.get_nearby_merchant_zone(client_id_);
    std::vector<ItemId> shop_items = merchant_catalog_for_zone(zone);

    std::string zone_name = (zone == 0) ? "Ciudad" : (zone == 1) ? "Pueblo" : "";
    std::string header = zone_name.empty() ? "=== Tienda del Comerciante ===\n" :
                                             "=== Tienda del Comerciante (" + zone_name + ") ===\n";

    std::string msg = header;
    for (ItemId iid: shop_items) {
        const ItemDef& def = Items::get(iid);
        uint32_t price = (static_cast<uint32_t>(def.min_value) + def.max_value) * 8 + 50;
        msg += def.name + " - " + std::to_string(price) + " oro\n";
    }
    world.push_message(client_id_, 0, msg);
}

bool MerchantCommands::try_list_catalog(World& world) {
    if (!world.player_near_service_npc(client_id_, NpcId::MERCHANT))
        return false;
    list_catalog(world);
    return true;
}
