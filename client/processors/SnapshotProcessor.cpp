#include "SnapshotProcessor.h"
#include <SDL2/SDL.h>
#include "../../common/protocol/protocol.h"
#include "../audio/GameAudioService.h"
#include "../config/AudioConfig.h"
#include "../ui/ChatWidget.h"
#include "../ui/StatsPanel.h"
#include "../ui/InventoryPanel.h"

SnapshotProcessor::SnapshotProcessor(GameAudioService& audio, StatsPanel* stats,
                                     InventoryPanel* inventory, ChatWidget* chat)
    : _audio(audio), _stats(stats), _inventory(inventory), _chat(chat) {}

void SnapshotProcessor::apply_map(WorldState& state, const MapaDTO& map) {
    state.map = map;
    state.map_loaded = true;
}

void SnapshotProcessor::apply_snapshot(WorldState& state, PlayerState& player, const SnapshotDTO& snap) {
    state.my_entity_id = snap.self_entity_id;
    state.current_tick = snap.tick;

    if (snap.entities) {
        detect_deaths(state, player, snap);
        update_entity_motion(state, *snap.entities);
        state.entities = *snap.entities;
    }

    update_progression_sounds(state, snap);
    update_service_npc_ranges(state, player);
    announce_chat_messages(snap);
    update_panels(snap);
    sync_own_equipment(state, snap);
    sync_own_position(state, player, snap);
}

void SnapshotProcessor::detect_deaths(WorldState& state, const PlayerState& player, const SnapshotDTO& snap) {
    for (const EntityDTO& prev : state.entities) {
        if (prev.entity_type != static_cast<uint8_t>(EntityType::NPC)) continue;

        bool found = false;
        for (const EntityDTO& next_entity : *snap.entities)
            if (next_entity.entity_id == prev.entity_id) { found = true; break; }
        if (found) continue;

        state.death_effects.push_back({prev.pos_x, prev.pos_y, SDL_GetTicks()});
        float dist = dist_to_player_tiles(player, prev.pos_x, prev.pos_y);
        _audio.npc_death(prev.sprite_id, prev.entity_id, prev.pos_y, dist, own_weapon_item(state));
    }

    for (const EntityDTO& next_entity : *snap.entities) {
        if (next_entity.entity_type != static_cast<uint8_t>(EntityType::PLAYER)) continue;
        if (next_entity.entity_id == state.my_entity_id || next_entity.is_ghost == 0) continue;

        for (const EntityDTO& prev : state.entities) {
            if (prev.entity_id == next_entity.entity_id && prev.is_ghost == 0) {
                _audio.player_death(dist_to_player_tiles(player, next_entity.pos_x, next_entity.pos_y));
                break;
            }
        }
    }
}

void SnapshotProcessor::update_service_npc_range(int32_t& npc_id, float range_tiles, const WorldState& state,
                                                  const PlayerState& player,
                                                  const std::function<void(float)>& on_farewell) {
    if (npc_id == -1) return;

    const EntityDTO* npc = nullptr;
    for (const EntityDTO& entity : state.entities)
        if (entity.entity_id == npc_id) { npc = &entity; break; }

    bool left = (npc == nullptr) || (dist_to_player_tiles(player, npc->pos_x, npc->pos_y) > range_tiles);
    if (!left) return;

    if (on_farewell) on_farewell(0.0f);
    npc_id = -1;
}

void SnapshotProcessor::update_service_npc_ranges(WorldState& state, const PlayerState& player) {
    AudioConfig::Interaction& interaction = AudioConfig::instance().interaction;
    update_service_npc_range(state.shop_npc_id, interaction.shop_range_tiles, state, player,
                              [this](float d) { _audio.merchant_farewell(d); });
    update_service_npc_range(state.bank_npc_id, interaction.bank_range_tiles, state, player, nullptr);
    update_service_npc_range(state.priest_npc_id, interaction.priest_range_tiles, state, player,
                              [this](float d) { _audio.priest_farewell(d); });
}

void SnapshotProcessor::announce_chat_messages(const SnapshotDTO& snap) {
    if (!snap.messages) return;
    for (const ChatMessageDTO& message : *snap.messages) {
        if (_chat) _chat->add_message(message.text);
        if (message.text.rfind("Compraste ", 0) == 0 || message.text.rfind("Vendiste ", 0) == 0)
            _audio.coins_received();
        else if (message.text.rfind("Fundaste el clan \"", 0) == 0)
            _audio.clan_created();
        else if (message.text.rfind("[Clan] ", 0) == 0)
            _audio.clan_member_attacked();
        else if (message.text.find(" → ti]: ") != std::string::npos)
            _audio.private_message_received();
        else if (message.text.rfind("Usaste Pocion de ", 0) == 0)
            _audio.potion_used();
        else if (message.text.rfind("Recibiste ", 0) == 0 &&
                 message.text.find(" de daño") != std::string::npos)
            _audio.damage_received(0.0f);
    }
}

void SnapshotProcessor::update_progression_sounds(WorldState& state, const SnapshotDTO& snap) {
    if (!state.spawned) _audio.player_spawn();
    state.spawned = true;

    if (snap.is_ghost != 0 && !state.was_ghost) _audio.player_death(0.0f);
    if (snap.is_ghost == 0 && state.was_ghost) {
        state.shop_npc_id = state.bank_npc_id = state.priest_npc_id = -1;
    }
    state.was_ghost = (snap.is_ghost != 0);

    _audio.update_meditation_loop(snap.meditating != 0);

    if (state.level_initialized && snap.level > state.last_level) _audio.level_up();
    state.last_level = snap.level;
    state.level_initialized = true;
}

void SnapshotProcessor::update_panels(const SnapshotDTO& snap) {
    if (_stats) {
        uint8_t equipped_weapon_item_id = 0;
        if (snap.equipped_weapon != 0xFF && snap.equipped_weapon < SnapshotDTO::INVENTORY_SIZE)
            equipped_weapon_item_id = snap.inventory[snap.equipped_weapon];
        _stats->update(snap.hp, snap.max_hp, snap.mp, snap.max_mp, snap.gold, snap.level, snap.exp,
                        snap.meditating != 0, snap.is_ghost != 0,
                        snap.character_class, equipped_weapon_item_id);
    }

    if (_inventory) {
        _inventory->update(snap.inventory, snap.equipped_weapon, snap.equipped_armor,
                           snap.equipped_helmet, snap.equipped_shield);
    }
}

void SnapshotProcessor::sync_own_equipment(WorldState& state, const SnapshotDTO& snap) {
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
        state.inventory[i] = snap.inventory[i];
    state.eq_weapon = snap.equipped_weapon;
    state.eq_armor  = snap.equipped_armor;
    state.eq_helmet = snap.equipped_helmet;
    state.eq_shield = snap.equipped_shield;
}

void SnapshotProcessor::sync_own_position(const WorldState& state, PlayerState& player, const SnapshotDTO& snap) {
    if (!snap.entities) return;
    for (const EntityDTO& entity : *snap.entities) {
        if (entity.entity_id != snap.self_entity_id) continue;
        if (entity.pos_x != static_cast<uint16_t>(player.tile_x) ||
            entity.pos_y != static_cast<uint16_t>(player.tile_y)) {
            Direction direction = static_cast<Direction>(entity.direction);
            player.move_to(entity.pos_x, entity.pos_y, direction);
            if (is_floor_grass(state, entity.pos_x, entity.pos_y) ||
                is_floor_dirt(state, entity.pos_x, entity.pos_y))
                _audio.footstep_grass();
        }
        break;
    }
}
