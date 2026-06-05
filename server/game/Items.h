#ifndef ITEMS_H
#define ITEMS_H

#include <cstdint>
#include <string>
#include "../../common/protocol/protocol.h"

enum class ItemKind : uint8_t {
    NONE, WEAPON_MELEE, WEAPON_RANGED, ARMOR, HELMET, SHIELD, POTION, GOLD
};

struct ItemDef {
    ItemId   id;
    ItemKind kind;
    std::string name;
    uint16_t min_value;
    uint16_t max_value;
    uint16_t mana_cost;
    uint16_t range_tiles;
};

namespace Items {
    const ItemDef& get(ItemId id);
    bool exists(ItemId id);
    EquipSlot equip_slot_for(ItemKind kind);
}

#endif
