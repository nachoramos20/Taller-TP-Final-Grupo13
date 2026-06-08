#include "WorldSnapshot.h"
#include "WorldPlayers.h"
#include "WorldNpcs.h"
#include "WorldItems.h"
#include "WorldChat.h"
#include "../Npc.h"

std::shared_ptr<std::vector<EntityDTO>> WorldSnapshot::get_entities() const {
    auto entities = std::make_shared<std::vector<EntityDTO>>();
    entities->reserve(players.all().size() + npcs.all().size() + items.all().size());

    for (const auto& [id, player] : players.all()) {
        EntityDTO e{};
        e.entity_id   = player.entity_id;
        e.entity_type = static_cast<uint8_t>(EntityType::PLAYER);
        e.username    = player.username;
        e.pos_x       = player.pos_x;
        e.pos_y       = player.pos_y;
        e.direction   = player.direction;
        e.sprite_id   = static_cast<uint8_t>(player.race + 1);
        e.is_ghost    = player.is_ghost ? 1 : 0;
        e.hp_pct      = static_cast<uint8_t>(
            player.max_hp > 0 ? (player.hp * 100) / player.max_hp : 0);
        entities->push_back(e);
    }

    for (const auto& npc : npcs.all()) {
        EntityDTO e{};
        e.entity_id   = npc.entity_id;
        e.entity_type = static_cast<uint8_t>(EntityType::NPC);
        e.username    = Npcs::tpl(npc.type).name;
        e.pos_x       = npc.pos_x;
        e.pos_y       = npc.pos_y;
        e.direction   = npc.direction;
        e.sprite_id   = static_cast<uint8_t>(npc.type);
        e.is_ghost    = 0;
        e.hp_pct      = static_cast<uint8_t>(
            npc.max_hp > 0 ? (npc.hp * 100) / npc.max_hp : 0);
        entities->push_back(e);
    }

    for (const auto& fi : items.all()) {
        EntityDTO e{};
        e.entity_id   = fi.entity_id;
        e.entity_type = static_cast<uint8_t>(EntityType::ITEM_FLOOR);
        e.pos_x       = fi.pos_x;
        e.pos_y       = fi.pos_y;
        e.sprite_id   = fi.item_id;
        entities->push_back(e);
    }

    return entities;
}

SnapshotDTO WorldSnapshot::build(uint16_t client_id,
                                 uint32_t tick,
                                 const std::shared_ptr<std::vector<EntityDTO>>& entities) const {
    SnapshotDTO snap{};
    snap.tick = tick;

    if (const PlayerData* p = players.find(client_id)) {
        snap.self_entity_id = p->entity_id;
        snap.hp             = p->hp;
        snap.max_hp         = p->max_hp;
        snap.mp             = p->mp;
        snap.max_mp         = p->max_mp;
        snap.exp            = p->exp;
        snap.level          = p->level;
        snap.gold           = p->gold;
        snap.cls            = p->cls;
        snap.is_ghost       = p->is_ghost ? 1 : 0;
        snap.meditating     = p->meditating ? 1 : 0;

        for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; ++i)
            snap.inventory[i] = p->inventory[i];

        snap.equipped_wpn  = p->equipped_weapon;
        snap.equipped_arm  = p->equipped_armor;
        snap.equipped_helm = p->equipped_helmet;
        snap.equipped_shld = p->equipped_shield;
    }

    snap.entities = entities;
    snap.messages = chat.collect(client_id);
    return snap;
}
