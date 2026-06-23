#ifndef ITEMS_H
#define ITEMS_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include "../../common/protocol/protocol.h"
#include "config/GameConfig.h"

// ItemDef — estructura usada en el resto del código del servidor.
// Los valores se cargan desde items.toml via GameConfig.
struct ItemDef {
    ItemId      id;
    ItemKind    kind;
    std::string name;
    uint16_t    min_value;    // daño mín (armas) / defensa mín (armaduras)
    uint16_t    max_value;    // daño máx / defensa máx
    uint16_t    mana_cost;    // solo weapon_magic
    uint16_t    range_tiles;  // solo ranged/magic
};

namespace Items {
    const ItemDef& get(ItemId id);
    bool           exists(ItemId id);
    EquipSlot      equip_slot_for(ItemKind kind);

    // Compara dos nombres de item ignorando mayúsculas/minúsculas.
    bool           name_equals_ci(const std::string& a, const std::string& b);
}

#endif
