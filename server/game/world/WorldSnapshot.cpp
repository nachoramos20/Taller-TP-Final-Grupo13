#include "WorldSnapshot.h"

#include "../Npc.h"

#include "WorldChat.h"
#include "WorldItems.h"
#include "WorldNpcs.h"
#include "WorldPlayers.h"

namespace {

uint8_t resolve_equipped_item(const PlayerData& player, uint8_t slot) {
    if (slot >= PlayerData::INVENTORY_SIZE)
        return 0;
    return player.inventory[slot];
}

}  // namespace

std::shared_ptr<std::vector<EntityDTO>> WorldSnapshot::get_entities() const {
    std::shared_ptr<std::vector<EntityDTO>> entities = std::make_shared<std::vector<EntityDTO>>();
    entities->reserve(players.all().size() + npcs.all().size() + items.all().size());

    for (const std::pair<const uint16_t, PlayerData>& player_entry: players.all()) {
        const PlayerData& player = player_entry.second;
        EntityDTO e{};
        e.entity_id = player.entity_id;
        e.entity_type = static_cast<uint8_t>(EntityType::PLAYER);
        e.username = player.username;
        e.pos_x = player.pos_x;
        e.pos_y = player.pos_y;
        e.direction = player.direction;
        e.sprite_id = static_cast<uint8_t>(player.race + 1);
        e.is_ghost = player.is_ghost ? 1 : 0;
        e.hp_pct = static_cast<uint8_t>(player.max_hp > 0 ? (player.hp * 100) / player.max_hp : 0);
        e.equipped_weapon = resolve_equipped_item(player, player.equipped_weapon);
        e.equipped_armor = resolve_equipped_item(player, player.equipped_armor);
        e.equipped_helmet = resolve_equipped_item(player, player.equipped_helmet);
        e.equipped_shield = resolve_equipped_item(player, player.equipped_shield);
        e.level = player.level;
        entities->push_back(e);
    }

    for (const NpcData& npc: npcs.all()) {
        EntityDTO e{};
        e.entity_id = npc.entity_id;
        e.entity_type = static_cast<uint8_t>(EntityType::NPC);
        e.username = Npcs::tpl(npc.type).name;
        e.pos_x = npc.pos_x;
        e.pos_y = npc.pos_y;
        e.direction = npc.direction;
        e.sprite_id = static_cast<uint8_t>(npc.type);
        e.is_ghost = 0;
        e.hp_pct = static_cast<uint8_t>(npc.max_hp > 0 ? (npc.hp * 100) / npc.max_hp : 0);
        entities->push_back(e);
    }

    for (const FloorItem& floor_item: items.all()) {
        EntityDTO e{};
        e.entity_id = floor_item.entity_id;
        e.entity_type = static_cast<uint8_t>(EntityType::ITEM_FLOOR);
        e.pos_x = floor_item.pos_x;
        e.pos_y = floor_item.pos_y;
        e.sprite_id = floor_item.item_id;
        e.direction = floor_item.sprite_variant;
        entities->push_back(e);
    }

    return entities;
}

SnapshotDTO WorldSnapshot::build(
        uint16_t client_id, uint32_t tick, const std::shared_ptr<std::vector<EntityDTO>>& entities,
        const std::shared_ptr<std::vector<SpellEventDTO>>& spell_events) const {
    SnapshotDTO snapshot{};
    snapshot.tick = tick;

    if (const PlayerData* p = players.find(client_id)) {
        snapshot.self_entity_id = p->entity_id;
        snapshot.hp = p->hp;
        snapshot.max_hp = p->max_hp;
        snapshot.mp = p->mp;
        snapshot.max_mp = p->max_mp;
        snapshot.exp = p->exp;
        snapshot.level = p->level;
        snapshot.gold = p->gold;
        snapshot.character_class = p->cls;
        snapshot.is_ghost = p->is_ghost ? 1 : 0;
        snapshot.meditating = p->meditating ? 1 : 0;

        for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; ++i)
            snapshot.inventory[i] = p->inventory[i];

        snapshot.equipped_weapon = p->equipped_weapon;
        snapshot.equipped_armor = p->equipped_armor;
        snapshot.equipped_helmet = p->equipped_helmet;
        snapshot.equipped_shield = p->equipped_shield;
    }

    snapshot.entities = entities;
    snapshot.messages = chat.collect(client_id);
    snapshot.spell_events = spell_events;
    return snapshot;
}
