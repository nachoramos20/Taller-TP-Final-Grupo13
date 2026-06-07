#include "Commands.h"
#include "../Items.h"
#include <algorithm>

EquipCommand::EquipCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void EquipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;
    if (inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t raw_id = p->inventory[inv_slot];
    if (raw_id == 0) return; // Slot vacío

    ItemId item_id = static_cast<ItemId>(raw_id);
    if (!Items::exists(item_id)) return;

    const ItemDef& def = Items::get(item_id);
    if (def.kind == ItemKind::NONE || def.kind == ItemKind::POTION ||
        def.kind == ItemKind::GOLD) return;

    if (static_cast<Class>(p->cls) == Class::WARRIOR &&
        def.kind == ItemKind::WEAPON_RANGED && def.mana_cost > 0) {
        world.push_message(client_id, 0, "El Guerrero no puede usar magia.");
        return;
    }

    EquipSlot slot = Items::equip_slot_for(def.kind);

    if (slot == EquipSlot::WEAPON) {
        bool new_is_magic   = (def.mana_cost > 0);
        bool curr_is_magic  = false;
        
        // Ahora p->equipped_weapon guarda el SLOT (0-15). Necesitamos verificar si ese slot es válido.
        // Usamos una constante mágica como 0xFF (255) para indicar "NADA EQUIPADO".
        if (p->equipped_weapon != 0xFF) {
            uint8_t curr_item_id = p->inventory[p->equipped_weapon];
            if (Items::exists(static_cast<ItemId>(curr_item_id))) {
                curr_is_magic = (Items::get(static_cast<ItemId>(curr_item_id)).mana_cost > 0);
            }
        }
        if (new_is_magic != curr_is_magic && p->equipped_weapon != 0xFF) {
            world.push_message(client_id, 0, "No puedes tener arma y báculo equipados a la vez.");
            return;
        }
    }

    // En lugar de guardar el "raw_id", guardamos el "inv_slot" (el índice de la mochila)
    // Si ya había algo equipado en ese slot corporal, simplemente se pisa por el nuevo slot elegido.
    switch (slot) {
        case EquipSlot::WEAPON: p->equipped_weapon = inv_slot; break;
        case EquipSlot::ARMOR:  p->equipped_armor  = inv_slot; break;
        case EquipSlot::HELMET: p->equipped_helmet = inv_slot; break;
        case EquipSlot::SHIELD: p->equipped_shield = inv_slot; break;
    }
    // YA NO HACEMOS: p->inventory[inv_slot] = prev; 
    // El ítem se queda intacto en la mochila.
}

UnequipCommand::UnequipCommand(uint16_t c, EquipSlot s) : client_id(c), slot(s) {}

void UnequipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    // Ya no necesitamos buscar un slot libre ni mover ítems de lugar.
    // Simplemente marcamos el slot del cuerpo como "vacío" usando 0xFF.
    switch (slot) {
        case EquipSlot::WEAPON: p->equipped_weapon = 0xFF; break;
        case EquipSlot::ARMOR:  p->equipped_armor  = 0xFF; break;
        case EquipSlot::HELMET: p->equipped_helmet = 0xFF; break;
        case EquipSlot::SHIELD: p->equipped_shield = 0xFF; break;
    }
}

DropCommand::DropCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void DropCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t item_id = p->inventory[inv_slot];
    if (item_id == 0) return;

    if (p->equipped_weapon == inv_slot) p->equipped_weapon = 0xFF;
    if (p->equipped_armor  == inv_slot) p->equipped_armor  = 0xFF;
    if (p->equipped_helmet == inv_slot) p->equipped_helmet = 0xFF;
    if (p->equipped_shield == inv_slot) p->equipped_shield = 0xFF;

    p->inventory[inv_slot] = 0;
    world.add_floor_item(item_id, p->pos_x, p->pos_y, 0);
}

PickCommand::PickCommand(uint16_t c) : client_id(c) {}

void PickCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        world.push_message(client_id, 0, "Inventario lleno.");
        return;
    }

    uint32_t gold_out = 0;
    uint8_t  item_id  = world.pick_floor_item(p->pos_x, p->pos_y, gold_out);

    if (item_id == static_cast<uint8_t>(ItemId::GOLD_PILE) && gold_out > 0) {
        p->gold += gold_out;
        world.push_message(client_id, 0, "Recogiste " + std::to_string(gold_out) + " de oro.");
        return;
    }
    if (item_id == 0) {
        world.push_message(client_id, 0, "No hay nada aquí.");
        return;
    }
    p->inventory[free_slot] = item_id;
    if (Items::exists(static_cast<ItemId>(item_id)))
        world.push_message(client_id, 0, "Recogiste: " + Items::get(static_cast<ItemId>(item_id)).name);
}

UseItemCommand::UseItemCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void UseItemCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost || inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t raw_id = p->inventory[inv_slot];
    if (raw_id == 0) return;

    ItemId item_id = static_cast<ItemId>(raw_id);
    if (!Items::exists(item_id)) return;

    const ItemDef& def = Items::get(item_id);
    if (def.kind != ItemKind::POTION) return;

    p->meditating = false;

    if (item_id == ItemId::HEALTH_POTION) {
        p->hp = std::min(p->max_hp, static_cast<uint16_t>(p->hp + def.min_value));
        world.push_message(client_id, 0, "Usaste Pocion de Vida. HP +" + std::to_string(def.min_value));
    } else if (item_id == ItemId::MANA_POTION) {
        p->mp = std::min(p->max_mp, static_cast<uint16_t>(p->mp + def.min_value));
        world.push_message(client_id, 0, "Usaste Pocion de Mana. MP +" + std::to_string(def.min_value));
    }
    p->inventory[inv_slot] = 0;
}
