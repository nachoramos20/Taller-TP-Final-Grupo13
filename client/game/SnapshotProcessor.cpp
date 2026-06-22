#include "SnapshotProcessor.h"
#include <SDL2/SDL.h>
#include "../../common/protocol/protocol.h"
#include "../audio/GameAudioService.h"
#include "../config/AudioConfig.h"
#include "ChatWidget.h"
#include "StatsPanel.h"
#include "InventoryPanel.h"

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
    // NPCs que estaban en el snapshot anterior y ya no están: murieron.
    for (const auto& prev : state.entities) {
        if (prev.entity_type != static_cast<uint8_t>(EntityType::NPC)) continue;

        bool found = false;
        for (const auto& ne : *snap.entities)
            if (ne.entity_id == prev.entity_id) { found = true; break; }
        if (found) continue;

        state.death_effects.push_back({prev.pos_x, prev.pos_y, SDL_GetTicks()});
        float dist = dist_to_player_tiles(player, prev.pos_x, prev.pos_y);
        _audio.npc_death(prev.sprite_id, prev.entity_id, prev.pos_y, dist, own_weapon_item(state));
    }

    // Otros jugadores que acaban de morir (pasan a fantasma). La muerte
    // propia se detecta en update_progression_sounds vía snap.is_ghost.
    for (const auto& ne : *snap.entities) {
        if (ne.entity_type != static_cast<uint8_t>(EntityType::PLAYER)) continue;
        if (ne.entity_id == state.my_entity_id || ne.is_ghost == 0) continue;

        for (const auto& prev : state.entities) {
            if (prev.entity_id == ne.entity_id && prev.is_ghost == 0) {
                _audio.player_death(dist_to_player_tiles(player, ne.pos_x, ne.pos_y));
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
    for (const auto& e : state.entities)
        if (e.entity_id == npc_id) { npc = &e; break; }

    bool left = (npc == nullptr) || (dist_to_player_tiles(player, npc->pos_x, npc->pos_y) > range_tiles);
    if (!left) return;

    if (on_farewell) on_farewell(0.0f);
    npc_id = -1;
}

void SnapshotProcessor::update_service_npc_ranges(WorldState& state, const PlayerState& player) {
    const auto& interaction = AudioConfig::instance().interaction;

    // El banco no tiene despedida: si vuelvo a tocarlo, recibo el saludo inicial de nuevo.
    update_service_npc_range(state.shop_npc_id, interaction.shop_range_tiles, state, player,
                              [this](float d) { _audio.merchant_farewell(d); });
    update_service_npc_range(state.bank_npc_id, interaction.bank_range_tiles, state, player, nullptr);
    update_service_npc_range(state.priest_npc_id, interaction.priest_range_tiles, state, player,
                              [this](float d) { _audio.priest_farewell(d); });
}

void SnapshotProcessor::announce_chat_messages(const SnapshotDTO& snap) {
    if (!snap.messages) return;
    for (const auto& m : *snap.messages) {
        if (_chat) _chat->add_message(m.text);
        if (m.text.rfind("Compraste ", 0) == 0 || m.text.rfind("Vendiste ", 0) == 0)
            _audio.coins_received();
        else if (m.text.rfind("Fundaste el clan \"", 0) == 0)
            _audio.clan_created();
        else if (m.text.rfind("[Clan] ", 0) == 0)
            _audio.clan_member_attacked();
        else if (m.text.find(" → ti]: ") != std::string::npos)
            _audio.private_message_received();
        else if (m.text.rfind("Usaste Pocion de ", 0) == 0)
            _audio.potion_used();
        else if (m.text.rfind("Recibiste ", 0) == 0 && m.text.find(" de daño") != std::string::npos)
            _audio.damage_received(0.0f);
    }
}

void SnapshotProcessor::update_progression_sounds(WorldState& state, const SnapshotDTO& snap) {
    if (!state.spawned) _audio.player_spawn();
    state.spawned = true;

    if (snap.is_ghost != 0 && !state.was_ghost) _audio.player_death(0.0f);
    if (snap.is_ghost == 0 && state.was_ghost) {
        // Resucité
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
        uint8_t eq_weapon_item = 0;
        if (snap.equipped_wpn != 0xFF && snap.equipped_wpn < SnapshotDTO::INVENTORY_SIZE)
            eq_weapon_item = snap.inventory[snap.equipped_wpn];
        _stats->update(snap.hp, snap.max_hp, snap.mp, snap.max_mp, snap.gold, snap.level, snap.exp,
                        snap.meditating != 0, snap.is_ghost != 0, snap.cls, eq_weapon_item);
    }

    if (_inventory) {
        _inventory->update(snap.inventory, snap.equipped_wpn, snap.equipped_arm,
                            snap.equipped_helm, snap.equipped_shld);
    }
}

void SnapshotProcessor::sync_own_equipment(WorldState& state, const SnapshotDTO& snap) {
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
        state.inventory[i] = snap.inventory[i];
    state.eq_weapon = snap.equipped_wpn;
    state.eq_armor  = snap.equipped_arm;
    state.eq_helmet = snap.equipped_helm;
    state.eq_shield = snap.equipped_shld;
}

void SnapshotProcessor::sync_own_position(const WorldState& state, PlayerState& player, const SnapshotDTO& snap) {
    if (!snap.entities) return;
    for (const auto& e : *snap.entities) {
        if (e.entity_id != snap.self_entity_id) continue;
        if (e.pos_x != static_cast<uint16_t>(player.tile_x) || e.pos_y != static_cast<uint16_t>(player.tile_y)) {
            Direction dir = static_cast<Direction>(e.direction);
            player.move_to(e.pos_x, e.pos_y, dir);
            if (is_floor_grass(state, e.pos_x, e.pos_y) || is_floor_dirt(state, e.pos_x, e.pos_y))
                _audio.footstep_grass();
        }
        break;
    }
}
