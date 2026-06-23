#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../common/protocol/dtos.h"
#include "../../../common/queue.h"
#include "../Npc.h"
#include "../entities/Clan.h"
#include "../entities/FloorItem.h"
#include "../entities/PlayerData.h"

#include "IdAllocator.h"
#include "Mazmorra.h"
#include "WorldBank.h"
#include "WorldChat.h"
#include "WorldClans.h"
#include "WorldCollision.h"
#include "WorldItems.h"
#include "WorldNpcs.h"
#include "WorldPlayers.h"
#include "WorldSnapshot.h"
#include "WorldSpawner.h"

// Façade del mundo. Mantiene la misma API pública que la versión monolítica
// previa; internamente delega en subsistemas cohesivos.
class World {
private:
    // El orden de declaración define el orden de construcción.
    IdAllocator id_alloc;
    std::mt19937 rng;
    WorldCollision collision_;
    WorldChat chat_;
    WorldPlayers players_;
    WorldItems items_;
    WorldClans clans_;
    WorldBank bank_;
    WorldNpcs npcs_;
    WorldSpawner spawner_;
    WorldSnapshot snapshot_;
    std::vector<Mazmorra> mazmorras_;

    std::unordered_map<uint16_t, NpcId> selected_service_npc_;

public:
    World(uint16_t width, uint16_t height, std::vector<uint8_t> collision_map,
          Queue<PlayerData>& save_queue);

    void add_player(const PlayerData& player_data);
    void remove_player(uint16_t client_id);
    void move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y);
    void tp_player(uint16_t client_id, uint16_t new_x, uint16_t new_y);

    // Sube de nivel a p tantas veces como la exp acumulada lo permita.
    // Se llama tanto desde
    // ataques cuerpo a cuerpo/distancia como desde hechizos, para no perder
    // niveles según el medio de ataque usado para matar.
    void check_level_up(PlayerData& p);

    const std::unordered_map<uint16_t, PlayerData>& get_players() const;
    const PlayerData* find_player(uint16_t client_id) const;
    PlayerData* get_player_mutable(uint16_t client_id);
    std::unordered_map<uint16_t, PlayerData>& get_players_mutable();

    SnapshotDTO build_snapshot(
            uint16_t client_id, uint32_t tick,
            const std::shared_ptr<std::vector<EntityDTO>>& entities,
            const std::shared_ptr<std::vector<SpellEventDTO>>& spell_events) const;
    std::shared_ptr<std::vector<EntityDTO>> get_entities() const;

    void update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied);

    void add_floor_item(uint8_t item_id, uint16_t x, uint16_t y, uint32_t gold = 0,
                        uint32_t spawn_tick = 0);
    uint8_t pick_floor_item(uint16_t x, uint16_t y, uint32_t& gold_out);
    void cleanup_items(uint32_t current_tick);

    void drop_player_loot(PlayerData& dead, uint32_t spawn_tick = 0);

    void spawn_npc(NpcId type, uint16_t x, uint16_t y);
    void spawn_npc_in_zone(NpcId type, uint16_t x, uint16_t y, uint8_t zone_id);
    void tick_npcs(uint32_t current_tick);
    NpcData* find_npc(uint16_t id);

    void push_message(uint16_t to_id, uint8_t type, const std::string& text);
    void push_broadcast(uint8_t type, const std::string& text);
    std::shared_ptr<std::vector<ChatMessageDTO>> collect_messages(uint16_t client_id);

    // Se agrega uno cuando un ataque/hechizo es procesado con éxito (no
    // rechazado por fair-play/rango/maná), para que los demás clientes vean
    // el VFX del atacante. Se llama una sola vez por tick en
    // ServerGameLoop::broadcast_snapshots() y se comparte entre los
    // snapshots de todos los clientes; clear_spell_events()
    // se llama una vez al final del tick.
    void push_spell_event(uint16_t caster_id, uint8_t spell_id, uint16_t target_x,
                          uint16_t target_y, bool is_magic_projectile);
    std::shared_ptr<std::vector<SpellEventDTO>> get_spell_events() const;
    void clear_spell_events();

    bool bank_deposit_item(uint16_t client_id, uint8_t inv_slot);
    bool bank_withdraw_item(uint16_t client_id, const std::string& item_name);
    bool bank_deposit_gold(uint16_t client_id, uint32_t amount);
    bool bank_withdraw_gold(uint16_t client_id, uint32_t amount);
    std::string bank_list(uint16_t client_id) const;

    bool clan_found(uint16_t founder_id, const std::string& clan_name);
    bool clan_join_request(uint16_t player_id, const std::string& clan_name);
    std::string clan_review(uint16_t founder_id) const;
    bool clan_accept(uint16_t founder_id, const std::string& nick);
    bool clan_reject(uint16_t founder_id, const std::string& nick);
    bool clan_ban(uint16_t founder_id, const std::string& nick);
    bool clan_kick(uint16_t founder_id, const std::string& nick);
    bool clan_leave(uint16_t player_id);
    bool same_clan(uint16_t a, uint16_t b) const;
    void restore_clan_membership(const PlayerData& p);
    void clan_notify_login(uint16_t player_id, bool online);
    void clan_notify_attack(uint16_t attacked_id);

    uint16_t find_player_by_name(const std::string& name) const;
    int rand_range(int lo, int hi);
    void clear_broadcast_messages();

    const std::vector<NpcData>& get_npcs() const;

    WorldSpawner& spawner();
    bool in_safe_zone(uint16_t x, uint16_t y) const;

    Mazmorra& add_dungeon(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    Mazmorra* get_dungeon_at(uint16_t x, uint16_t y);
    std::vector<Mazmorra>& dungeons() { return mazmorras_; }
    const std::vector<Mazmorra>& dungeons() const { return mazmorras_; }

    void set_selected_npc(uint16_t client_id, NpcId type);
    NpcId get_selected_npc(uint16_t client_id) const;
    void clear_selected_npc(uint16_t client_id);
    bool player_near_service_npc(uint16_t client_id, NpcId required_type) const;
    uint8_t get_nearby_merchant_zone(uint16_t client_id) const;  // 255 = ninguno

    // Posición del sacerdote cercano al jugador: false si no hay ninguno
    // a distancia de interacción. Usado para resucitar junto al sacerdote
    // correcto, no a un punto fijo.
    bool find_nearby_priest_pos(uint16_t client_id, uint16_t& out_x, uint16_t& out_y) const;
};

#endif  // WORLD_H
