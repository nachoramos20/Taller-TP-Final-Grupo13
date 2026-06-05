#include "Commands.h"
#include "Items.h"
#include <algorithm>
#include <iostream>

// Move 
MoveCommand::MoveCommand(uint16_t c, uint16_t x, uint16_t y)
    : client_id(c), new_x(x), new_y(y) {}

void MoveCommand::execute(World& world) {
    world.move_player(client_id, new_x, new_y);
}

// Login
LoginCommand::LoginCommand(PlayerData p) : player_data(std::move(p)) {}

void LoginCommand::execute(World& world) {
    world.add_player(player_data);
}

const std::string& LoginCommand::get_username() const {
    return player_data.username;
}

// Attack 
AttackCommand::AttackCommand(uint16_t c, uint16_t t)
    : client_id(c), target_id(t) {}

void AttackCommand::execute(World& world) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    PlayerData* target   = world.get_player_mutable(target_id);

    if (!attacker || !target) return;
    if (attacker->is_ghost || target->is_ghost) return;
    if (attacker->attack_cooldown > 0) return;

    int dist_x = std::abs(static_cast<int>(attacker->pos_x) -
                          static_cast<int>(target->pos_x));
    int dist_y = std::abs(static_cast<int>(attacker->pos_y) -
                          static_cast<int>(target->pos_y));
    if (dist_x > 2 || dist_y > 2) return;

    attacker->meditating = false;

    uint16_t damage  = 5 + (attacker->strength / 2);
    uint16_t defense = target->agility / 3 + target->level;
    damage = (damage > defense) ? damage - defense : 1;

    if (target->hp > damage) {
        target->hp -= damage;
    } else {
        target->hp       = 0;
        target->is_ghost = true;
        target->meditating = false;
        attacker->exp   += target->level * 25;
        world.update_occupied({target->pos_x, target->pos_y}, false);
    }

    attacker->attack_cooldown = 10;
}

// Equip
EquipCommand::EquipCommand(uint16_t c, uint8_t s)
    : client_id(c), inv_slot(s) {}

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

    EquipSlot slot = Items::equip_slot_for(def.kind);
    switch (slot) {
        case EquipSlot::WEAPON: p->equipped_weapon = raw_id; break;
        case EquipSlot::ARMOR:  p->equipped_armor  = raw_id; break;
        case EquipSlot::HELMET: p->equipped_helmet = raw_id; break;
        case EquipSlot::SHIELD: p->equipped_shield = raw_id; break;
    }
    p->inventory[inv_slot] = 0;
}

// Unequip 
UnequipCommand::UnequipCommand(uint16_t c, EquipSlot s)
    : client_id(c), slot(s) {}

void UnequipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) { free_slot = i; break; }
    }
    if (free_slot == -1) return;

    uint8_t item_id = 0;
    switch (slot) {
        case EquipSlot::WEAPON:
            item_id = p->equipped_weapon; p->equipped_weapon = 0; break;
        case EquipSlot::ARMOR:
            item_id = p->equipped_armor;  p->equipped_armor  = 0; break;
        case EquipSlot::HELMET:
            item_id = p->equipped_helmet; p->equipped_helmet = 0; break;
        case EquipSlot::SHIELD:
            item_id = p->equipped_shield; p->equipped_shield = 0; break;
    }
    if (item_id != 0)
        p->inventory[free_slot] = item_id;
}

// Drop 
DropCommand::DropCommand(uint16_t c, uint8_t s)
    : client_id(c), inv_slot(s) {}

void DropCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t item_id = p->inventory[inv_slot];
    if (item_id == 0) return;

    p->inventory[inv_slot] = 0;
    world.add_floor_item(item_id, p->pos_x, p->pos_y);
}

// Pick 
PickCommand::PickCommand(uint16_t c) : client_id(c) {}

void PickCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) { free_slot = i; break; }
    }
    if (free_slot == -1) return;

    uint8_t item_id = world.pick_floor_item(p->pos_x, p->pos_y);
    if (item_id == 0) return;

    p->inventory[free_slot] = item_id;
}

// UseItem
UseItemCommand::UseItemCommand(uint16_t c, uint8_t s)
    : client_id(c), inv_slot(s) {}

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
        p->hp = std::min(p->max_hp,
            static_cast<uint16_t>(p->hp + def.min_value));
    } else if (item_id == ItemId::MANA_POTION) {
        p->mp = std::min(p->max_mp,
            static_cast<uint16_t>(p->mp + def.min_value));
    }
    p->inventory[inv_slot] = 0;
}

// Meditate 
MeditateCommand::MeditateCommand(uint16_t c) : client_id(c) {}

void MeditateCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    p->meditating = !p->meditating;
    if (p->meditating) {
        p->mp = std::min(p->max_mp,
            static_cast<uint16_t>(p->mp + (p->intelligence / 2)));
    }
}

// Resurrect 
ResurrectCommand::ResurrectCommand(uint16_t c) : client_id(c) {}

void ResurrectCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || !p->is_ghost) return;

    p->is_ghost    = false;
    p->meditating  = false;
    p->hp          = p->max_hp / 4;
    p->mp          = 0;
    world.update_occupied({p->pos_x, p->pos_y}, true);
}

// Logout 
LogoutCommand::LogoutCommand(uint16_t c) : client_id(c) {}

void LogoutCommand::execute(World& world) {
    world.remove_player(client_id);
}

// NpcInteract 
NpcInteractCommand::NpcInteractCommand(uint16_t c, uint16_t n)
    : client_id(c), npc_id(n) {}

void NpcInteractCommand::execute(World& world) {
    // por ahora no hace nada — se implementa cuando estén los NPCs en World
    (void)world;
}

// Chat 
ChatCommand::ChatCommand(uint16_t c, std::string cmd)
    : client_id(c), cmd(std::move(cmd)) {}

void ChatCommand::execute(World& world) {
    // por ahora no hace nada — se implementa con el sistema de chat
    (void)world;
}