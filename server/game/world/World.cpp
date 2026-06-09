#include "World.h"

World::World(uint16_t width, uint16_t height)
    : id_alloc(10000),
      rng(std::random_device{}()),
      collision_(width, height),
      chat_(),
      players_(collision_),
      items_(id_alloc),
      clans_(players_, chat_),
      bank_(players_, chat_),
      npcs_(collision_, players_, items_, chat_, clans_, id_alloc, rng, spawner_),
      spawner_(collision_, rng),
      snapshot_(players_, npcs_, items_, chat_) {}


// ---- Players ----
void World::add_player(const PlayerData& p) {
    players_.add(p);
    restore_clan_membership(p);
    clans_.notify_login(p.entity_id, true);
}
void World::remove_player(uint16_t id) {
    clans_.notify_login(id, false);
    players_.remove(id);
}
void World::move_player(uint16_t id, uint16_t x, uint16_t y) { players_.move(id, x, y); }

const std::unordered_map<uint16_t, PlayerData>& World::get_players() const { return players_.all(); }
std::unordered_map<uint16_t, PlayerData>& World::get_players_mutable() { return players_.all_mutable(); }
const PlayerData* World::find_player(uint16_t id) const { return players_.find(id); }
PlayerData* World::get_player_mutable(uint16_t id) { return players_.find_mutable(id); }

// ---- Snapshot ----
SnapshotDTO World::build_snapshot(uint16_t client_id, uint32_t tick,
                                  const std::shared_ptr<std::vector<EntityDTO>>& entities) const {
    return snapshot_.build(client_id, tick, entities);
}
std::shared_ptr<std::vector<EntityDTO>> World::get_entities() const { return snapshot_.get_entities(); }

// ---- Colisiones ----
void World::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied) {
    collision_.update_occupied(pos, occupied);
}
void World::revisar_colisiones() { collision_.revisar(players_, npcs_); }

// ---- Items ----
void World::add_floor_item(uint8_t item_id, uint16_t x, uint16_t y, uint32_t gold) {
    items_.add(item_id, x, y, gold);
}
uint8_t World::pick_floor_item(uint16_t x, uint16_t y, uint32_t& gold_out) {
    return items_.pick(x, y, gold_out);
}
void World::drop_player_loot(PlayerData& dead) { items_.drop_player_loot(dead); }

// ---- NPCs ----
void World::spawn_npc(NpcId type, uint16_t x, uint16_t y) { npcs_.spawn(type, x, y); }
void World::tick_npcs() { npcs_.tick(); }
NpcData* World::find_npc(uint16_t id) { return npcs_.find(id); }

// ---- Chat ----
void World::push_message(uint16_t to_id, uint8_t type, const std::string& text) {
    chat_.push_message(to_id, type, text);
}
void World::push_broadcast(uint8_t type, const std::string& text) {
    chat_.push_broadcast(type, text);
}
std::shared_ptr<std::vector<ChatMessageDTO>> World::collect_messages(uint16_t client_id) {
    return chat_.collect(client_id);
}
void World::clear_broadcast_messages() { chat_.clear_broadcasts(); }

// ---- Banco ----
bool World::bank_deposit_item(uint16_t id, uint8_t slot) { return bank_.deposit_item(id, slot); }
bool World::bank_withdraw_item(uint16_t id, const std::string& n) { return bank_.withdraw_item(id, n); }
bool World::bank_deposit_gold(uint16_t id, uint32_t amt) { return bank_.deposit_gold(id, amt); }
bool World::bank_withdraw_gold(uint16_t id, uint32_t amt) { return bank_.withdraw_gold(id, amt); }
std::string World::bank_list(uint16_t id) const { return bank_.list(id); }

// ---- Clanes ----
bool World::clan_found(uint16_t f, const std::string& n) { return clans_.found(f, n); }
bool World::clan_join_request(uint16_t p, const std::string& n) { return clans_.join_request(p, n); }
std::string World::clan_review(uint16_t f) const { return clans_.review(f); }
bool World::clan_accept(uint16_t f, const std::string& n) { return clans_.accept(f, n); }
bool World::clan_reject(uint16_t f, const std::string& n) { return clans_.reject(f, n); }
bool World::clan_ban(uint16_t f, const std::string& n) { return clans_.ban(f, n); }
bool World::clan_kick(uint16_t f, const std::string& n) { return clans_.kick(f, n); }
bool World::clan_leave(uint16_t p) { return clans_.leave(p); }
bool World::same_clan(uint16_t a, uint16_t b) const { return clans_.same_clan(a, b); }
void World::restore_clan_membership(const PlayerData& p) {
    if (p.clan_name[0] != '\0') {
        clans_.restore_membership(p.entity_id, std::string(p.clan_name), p.is_clan_founder);
    }
}
void World::clan_notify_login(uint16_t p, bool online) { clans_.notify_login(p, online); }
void World::clan_notify_attack(uint16_t a) { clans_.notify_attack(a); }

// ---- Helpers ----
uint16_t World::find_player_by_name(const std::string& name) const {
    return players_.find_by_name(name);
}
int World::rand_range(int lo, int hi) {
    if (lo >= hi) return lo;
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}
