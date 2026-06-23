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

    if (static_cast<Class>(p->cls) == Class::WARRIOR && def.mana_cost > 0) {
        world.push_message(client_id, 0, "El Guerrero no puede usar magia.");
        return;
    }

    EquipSlot slot = Items::equip_slot_for(def.kind);

    // No se aplica tabla/Factory en este switch ni en el de
    // UnequipCommand::execute: EquipSlot es un conjunto cerrado de 4 slots
    // de personaje (arma/armadura/casco/escudo) fijado por el diseño del
    // juego, no un catálogo de datos que vaya a crecer.
    switch (slot) {
        case EquipSlot::WEAPON: p->equipped_weapon = inv_slot; break;
        case EquipSlot::ARMOR:  p->equipped_armor  = inv_slot; break;
        case EquipSlot::HELMET: p->equipped_helmet = inv_slot; break;
        case EquipSlot::SHIELD: p->equipped_shield = inv_slot; break;
    }
}

UnequipCommand::UnequipCommand(uint16_t c, EquipSlot s) : client_id(c), slot(s) {}

void UnequipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    // Marcamos el slot del cuerpo como "vacío" usando 0xFF.
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

MoveItemCommand::MoveItemCommand(uint16_t c, uint8_t from, uint8_t to)
    : client_id(c), from_slot(from), to_slot(to) {}

void MoveItemCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    if (from_slot >= PlayerData::INVENTORY_SIZE || to_slot >= PlayerData::INVENTORY_SIZE) return;
    if (from_slot == to_slot) return;
    if (p->inventory[from_slot] == 0) return;  // nada para mover

    std::swap(p->inventory[from_slot], p->inventory[to_slot]);

    // Si alguno de los dos slots estaba equipado, el equipo se mueve con el item.
    auto remap = [&](uint8_t& equipped_slot) {
        if (equipped_slot == from_slot) equipped_slot = to_slot;
        else if (equipped_slot == to_slot) equipped_slot = from_slot;
    };
    remap(p->equipped_weapon);
    remap(p->equipped_armor);
    remap(p->equipped_helmet);
    remap(p->equipped_shield);
}

PickCommand::PickCommand(uint16_t c) : client_id(c) {}

void PickCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    uint32_t gold_out = 0;
    uint8_t  item_id  = world.pick_floor_item(p->pos_x, p->pos_y, gold_out);

    if (item_id == static_cast<uint8_t>(ItemId::GOLD_PILE)) {
        p->gold += gold_out;
        world.push_message(client_id, 0, "Recogiste " + std::to_string(gold_out) + " de oro.");
        return;
    }
    if (item_id == 0) {
        world.push_message(client_id, 0, "No hay nada aquí.");
        return;
    }

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        world.add_floor_item(item_id, p->pos_x, p->pos_y, 0);
        world.push_message(client_id, 0, "Inventario lleno.");
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
