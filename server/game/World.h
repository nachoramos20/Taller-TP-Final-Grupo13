#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>
#include <string>
#include <random>

#include "../../common/protocol/dtos.h"
#include "PlayerData.h"
#include "FloorItem.h"
#include "Npc.h"
#include "Clan.h"

namespace std {
    template <>
    struct hash<std::pair<uint16_t, uint16_t>> {
        size_t operator()(const std::pair<uint16_t, uint16_t>& p) const {
            return hash<uint16_t>()(p.first) ^ (hash<uint16_t>()(p.second) << 1);
        }
    };
}

// Mensajes de chat para enviar a un jugador en el próximo snapshot
struct PendingMessage {
    uint16_t    to_client_id;   // 0 = broadcast
    uint8_t     msg_type;       // 0=info, 1=combat, 2=private
    std::string text;
};

class World {
private:
    uint16_t width;
    uint16_t height;
    std::unordered_map<std::pair<uint16_t, uint16_t>, bool> occupied_positions;
    std::unordered_map<uint16_t, PlayerData> players_map;
    std::vector<FloorItem>  floor_items;
    std::vector<NpcData>    npcs;
    uint16_t                next_entity_id = 10000; // IDs de NPCs arrancan acá

    // Banco: bank[username][item_id] = cantidad
    std::unordered_map<std::string, std::vector<uint8_t>> bank_inventories;
    std::unordered_map<std::string, uint32_t>             bank_gold;

    // Clanes
    std::unordered_map<std::string, Clan>  clans_by_name;
    std::unordered_map<uint16_t, std::string> player_clan; // player_id -> clan_name

    // Mensajes pendientes (acumulados durante el tick)
    std::vector<PendingMessage> pending_messages;

    std::mt19937 rng;

    void set_direction_from_delta(PlayerData& player, int dx, int dy);
    uint16_t new_entity_id() { return next_entity_id++; }

public:
    World(uint16_t width, uint16_t height);

    // ---- Players ----
    void add_player(const PlayerData& player_data);
    void remove_player(uint16_t client_id);
    void move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y);

    const std::unordered_map<uint16_t, PlayerData>& get_players() const;
    const PlayerData* find_player(uint16_t client_id) const;
    PlayerData* get_player_mutable(uint16_t client_id);
    std::unordered_map<uint16_t, PlayerData>& get_players_mutable();

    // ---- Snapshot ----
    SnapshotDTO build_snapshot(uint16_t client_id,
                               uint32_t tick,
                               const std::shared_ptr<std::vector<EntityDTO>>& entities) const;
    std::shared_ptr<std::vector<EntityDTO>> get_entities() const;

    // ---- Colisiones ----
    void update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied);
    void revisar_colisiones();

    // ---- Items en el suelo ----
    void add_floor_item(uint8_t item_id, uint16_t x, uint16_t y, uint32_t gold = 0);
    uint8_t pick_floor_item(uint16_t x, uint16_t y, uint32_t& gold_out);

    // ---- Muerte de jugador: droppea inventario y oro ----
    void drop_player_loot(PlayerData& dead);

    // ---- NPCs ----
    void spawn_npc(NpcId type, uint16_t x, uint16_t y);
    void tick_npcs();               // movimiento + ataque
    NpcData* find_npc(uint16_t id);

    // ---- Mensajes / Chat ----
    void push_message(uint16_t to_id, uint8_t type, const std::string& text);
    void push_broadcast(uint8_t type, const std::string& text);
    std::shared_ptr<std::vector<ChatMessageDTO>> collect_messages(uint16_t client_id);

    // ---- Banco ----
    bool bank_deposit_item(uint16_t client_id, uint8_t inv_slot);
    bool bank_withdraw_item(uint16_t client_id, const std::string& item_name);
    bool bank_deposit_gold(uint16_t client_id, uint32_t amount);
    bool bank_withdraw_gold(uint16_t client_id, uint32_t amount);
    std::string bank_list(uint16_t client_id) const;

    // ---- Clanes ----
    bool clan_found(uint16_t founder_id, const std::string& clan_name);
    bool clan_join_request(uint16_t player_id, const std::string& clan_name);
    std::string clan_review(uint16_t founder_id) const;
    bool clan_accept(uint16_t founder_id, const std::string& nick);
    bool clan_reject(uint16_t founder_id, const std::string& nick);
    bool clan_ban(uint16_t founder_id, const std::string& nick);
    bool clan_kick(uint16_t founder_id, const std::string& nick);
    bool clan_leave(uint16_t player_id);
    bool same_clan(uint16_t a, uint16_t b) const;
    void clan_notify_login(uint16_t player_id, bool online);
    void clan_notify_attack(uint16_t attacked_id);

    // ---- Helpers ----
    uint16_t find_player_by_name(const std::string& name) const;
    int rand_range(int lo, int hi);
    void clear_broadcast_messages();

    const std::vector<NpcData>& get_npcs() const { return npcs; }
};

#endif // WORLD_H
