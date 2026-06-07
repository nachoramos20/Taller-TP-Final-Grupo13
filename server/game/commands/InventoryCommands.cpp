#include "Commands.h"
#include "../Items.h"
#include <algorithm>

EquipCommand::EquipCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void EquipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;
    if (inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t raw_id = p->inventory[inv_slot];
    if (raw_id == 0) return;

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
        if (p->equipped_weapon != 0 && Items::exists(static_cast<ItemId>(p->equipped_weapon)))
            curr_is_magic = (Items::get(static_cast<ItemId>(p->equipped_weapon)).mana_cost > 0);
        if (new_is_magic != curr_is_magic && p->equipped_weapon != 0) {
            world.push_message(client_id, 0, "No puedes tener arma y báculo equipados a la vez.");
            return;
        }
    }

    uint8_t prev = 0;
    switch (slot) {
        case EquipSlot::WEAPON: prev = p->equipped_weapon; p->equipped_weapon = raw_id; break;
        case EquipSlot::ARMOR:  prev = p->equipped_armor;  p->equipped_armor  = raw_id; break;
        case EquipSlot::HELMET: prev = p->equipped_helmet; p->equipped_helmet = raw_id; break;
        case EquipSlot::SHIELD: prev = p->equipped_shield; p->equipped_shield = raw_id; break;
    }
    p->inventory[inv_slot] = prev; 
}

UnequipCommand::UnequipCommand(uint16_t c, EquipSlot s) : client_id(c), slot(s) {}

void UnequipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        world.push_message(client_id, 0, "Inventario lleno, no puedes desequipar.");
        return;
    }

    uint8_t item_id = 0;
    switch (slot) {
        case EquipSlot::WEAPON: item_id = p->equipped_weapon; p->equipped_weapon = 0; break;
        case EquipSlot::ARMOR:  item_id = p->equipped_armor;  p->equipped_armor  = 0; break;
        case EquipSlot::HELMET: item_id = p->equipped_helmet; p->equipped_helmet = 0; break;
        case EquipSlot::SHIELD: item_id = p->equipped_shield; p->equipped_shield = 0; break;
    }
    if (item_id != 0)
        p->inventory[free_slot] = item_id;
}

DropCommand::DropCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void DropCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t item_id = p->inventory[inv_slot];
    if (item_id == 0) return;

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
