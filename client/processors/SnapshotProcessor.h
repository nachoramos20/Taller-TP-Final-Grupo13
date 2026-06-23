#pragma once

#include <functional>

#include "../../common/protocol/MapaDTO.h"
#include "../../common/protocol/dtos.h"
#include "../PlayerState.h"
#include "../WorldState.h"

class GameAudioService;
class StatsPanel;
class InventoryPanel;
class ChatWidget;

// Interpreta los SnapshotDTO/MapaDTO que llegan del servidor: actualiza
// WorldState y los paneles de UI, y dispara (vía GameAudioService) los
// sonidos correspondientes a transiciones de estado: muerte de NPC/jugador,
// nivel, meditación, compra/venta, y fin de interacción con un NPC de
// servicio (antes triplicado casi idéntico para tienda/banco/sacerdote).
class SnapshotProcessor {
public:
    SnapshotProcessor(GameAudioService& audio, StatsPanel* stats, InventoryPanel* inventory,
                      ChatWidget* chat);

    void apply_map(WorldState& state, const MapaDTO& map);
    void apply_snapshot(WorldState& state, PlayerState& player, const SnapshotDTO& snap);

private:
    void detect_deaths(WorldState& state, const PlayerState& player, const SnapshotDTO& snap);

    // VFX de ataque/hechizo de OTROS jugadores (el propio ya lo spawneó
    // PlayerActionController de forma optimista al clickear). Busca al
    // caster en state.entities (ya actualizado a este snapshot) para saber
    // desde dónde sale el proyectil; si no está visible, no hay "desde
    // dónde" dibujarlo y se descarta ese evento puntual.
    void apply_spell_events(WorldState& state, const SnapshotDTO& snap);
    void update_service_npc_ranges(WorldState& state, const PlayerState& player);
    void update_service_npc_range(int32_t& npc_id, float range_tiles, const WorldState& state,
                                  const PlayerState& player,
                                  const std::function<void(float)>& on_farewell);
    void announce_chat_messages(const SnapshotDTO& snap);
    void update_progression_sounds(WorldState& state, const SnapshotDTO& snap);
    void update_panels(const SnapshotDTO& snap);
    void sync_own_equipment(WorldState& state, const SnapshotDTO& snap);
    void sync_own_position(const WorldState& state, PlayerState& player, const SnapshotDTO& snap);

    GameAudioService& _audio;
    StatsPanel* _stats;
    InventoryPanel* _inventory;
    ChatWidget* _chat;
};
