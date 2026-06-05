#include "Game.h"
#include "Items.h"
#include <algorithm>
#include <iostream>


Game::Game() : world(100, 100) {}

void Game::add_player(const PlayerData& player_data) { world.add_player(player_data); }
void Game::remove_player(uint16_t client_id)         { world.remove_player(client_id); }
void Game::move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y) {
    world.move_player(client_id, new_x, new_y);
}
void Game::revisar_colisiones() { world.revisar_colisiones(); }

const std::unordered_map<uint16_t, PlayerData>& Game::get_players() const {
    return world.get_players();
}

std::shared_ptr<std::vector<EntityDTO>> Game::get_entities() const {
    return world.get_entities();
}

void Game::player_attack(uint16_t client_id, uint16_t target_id) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    PlayerData* target = world.get_player_mutable(target_id);
    
    if (!attacker || !target) return;
    if (attacker->is_ghost || target->is_ghost) return;

    if (attacker->attack_cooldown > 0) return;

    int dist_x = std::abs(static_cast<int>(attacker->pos_x) - static_cast<int>(target->pos_x));
    int dist_y = std::abs(static_cast<int>(attacker->pos_y) - static_cast<int>(target->pos_y));
    if (dist_x > 2 || dist_y > 2) return;

    attacker->meditating = false;

    uint16_t damage = 5 + (attacker->strength / 2);

    uint16_t defense = target->agility / 3 + target->level;
    if (damage > defense) {
        damage -= defense;
    } else {
        damage = 1;
    }

    if (target->hp > damage) {
        target->hp -= damage;
    } else {
        target->hp = 0;
        target->is_ghost = true;
        target->meditating = false;
        attacker->exp += target->level * 25;
        
        world.update_occupied({target->pos_x, target->pos_y}, false);
    }

    attacker->attack_cooldown = 10;
}

void Game::player_equip(uint16_t client_id, uint8_t inv_slot) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;
    if (inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t raw_id = p->inventory[inv_slot];
    if (raw_id == 0) return;

    ItemId item_id = static_cast<ItemId>(raw_id);
    if (!Items::exists(item_id)) return;

    const ItemDef& def = Items::get(item_id);
    EquipSlot slot = Items::equip_slot_for(def.kind);

    switch (slot) {
        case EquipSlot::WEAPON: p->equipped_weapon = raw_id; break;
        case EquipSlot::ARMOR:  p->equipped_armor  = raw_id; break;
        case EquipSlot::HELMET: p->equipped_helmet = raw_id; break;
        case EquipSlot::SHIELD: p->equipped_shield = raw_id; break;
    }

    p->inventory[inv_slot] = 0;
}



void Game::player_unequip(uint16_t client_id, EquipSlot slot) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) {
            free_slot = i;
            break;
        }
    }
    if (free_slot == -1) return;

    uint8_t item_id = 0;
    if (slot == EquipSlot::WEAPON && p->equipped_weapon != 0) {
        item_id = p->equipped_weapon;
        p->equipped_weapon = 0;
    } else if (slot == EquipSlot::ARMOR && p->equipped_armor != 0) {
        item_id = p->equipped_armor;
        p->equipped_armor = 0;
    } else if (slot == EquipSlot::SHIELD && p->equipped_shield != 0) {
        item_id = p->equipped_shield;
        p->equipped_shield = 0;
    } else if (slot == EquipSlot::HELMET && p->equipped_helmet != 0) {
        item_id = p->equipped_helmet;
        p->equipped_helmet = 0;
    }

    if (item_id != 0) {
        p->inventory[free_slot] = item_id;
    }
}

void Game::player_drop(uint16_t client_id, uint8_t inv_slot) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t item_id = p->inventory[inv_slot];
    if (item_id == 0) return;

    p->inventory[inv_slot] = 0;

    std::cout << "Player " << client_id << " tiro el item " << static_cast<int>(item_id) 
              << " en (" << p->pos_x << "," << p->pos_y << ")\n";
}

void Game::player_pick(uint16_t client_id) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) {
            free_slot = i;
            break;
        }
    }
    if (free_slot == -1) return;

    uint8_t item_del_suelo = 12; 
    p->inventory[free_slot] = item_del_suelo;
}

void Game::player_use(uint16_t client_id, uint8_t inv_slot) {
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

void Game::player_meditate(uint16_t client_id) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    p->meditating = !p->meditating; 

    if (p->meditating) {
        p->mp = std::min(p->max_mp, static_cast<uint16_t>(p->mp + (p->intelligence / 2)));
    }
}

void Game::player_resurrect(uint16_t client_id) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || !p->is_ghost) return;

    p->is_ghost = false;
    p->meditating = false;
    p->hp = p->max_hp / 4;
    p->mp = 0;

    world.update_occupied({p->pos_x, p->pos_y}, true);
}

SnapshotDTO Game::build_snapshot(uint16_t client_id,
                                 uint32_t tick,
                                 const std::shared_ptr<std::vector<EntityDTO>>& entities) const {
    SnapshotDTO snap{};
    snap.tick = tick;

    if (const PlayerData* p = world.find_player(client_id)) {
        snap.self_entity_id = p->entity_id;
        snap.hp             = p->hp;
        snap.max_hp         = p->max_hp;
        snap.mp             = p->mp;
        snap.max_mp         = p->max_mp;
        snap.exp            = p->exp;
        snap.level          = p->level;
        snap.gold           = p->gold;
        snap.is_ghost       = p->is_ghost ? 1 : 0;
        snap.meditating     = p->meditating ? 1 : 0;

        for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; ++i) {
            snap.inventory[i] = p->inventory[i];
        }

        snap.equipped_wpn  = p->equipped_weapon;
        snap.equipped_arm  = p->equipped_armor;
        snap.equipped_helm = p->equipped_helmet;
        snap.equipped_shld = p->equipped_shield;
    }

    snap.entities = entities;
    return snap;
}