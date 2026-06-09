#ifndef ITEMS_H
#define ITEMS_H

#include <cstdint>
#include <string>
#include "../../common/protocol/protocol.h"

enum class ItemKind : uint8_t {
    NONE,
    WEAPON_MELEE,
    WEAPON_RANGED,
    WEAPON_MAGIC,   // armas mágicas: consumen mana en ataque básico y habilitan hechizos
    ARMOR,
    HELMET,
    SHIELD,
    POTION,
    GOLD
};

struct ItemDef {
    ItemId   id;
    ItemKind kind;
    std::string name;
    uint16_t min_value;   // daño mínimo (armas) o defensa mínima (armaduras/esc/casco)
    uint16_t max_value;   // daño máximo
    uint16_t mana_cost;   // mana consumido en ataque BÁSICO (0 si no aplica)
    uint16_t range_tiles; // alcance en tiles
};

namespace Items {
    const ItemDef& get(ItemId id);
    bool exists(ItemId id);
    EquipSlot equip_slot_for(ItemKind kind);
}

#endif
