#include "World.h"

#include <algorithm>

#include "../Equations.h"
#include "../config/GameConfig.h"

World::World(uint16_t width, uint16_t height, std::vector<uint8_t> collision_map,
             Queue<PlayerData>& save_queue):
        id_alloc(10000),
        rng(std::random_device{}()),
        collision_(width, height, std::move(collision_map)),
        chat_(),
        players_(collision_, save_queue),
        items_(id_alloc),
        clans_(players_, chat_),
        bank_(players_, chat_),
        npcs_(collision_, players_, items_, chat_, clans_, id_alloc, rng, spawner_),
        spawner_(collision_, rng),
        snapshot_(players_, npcs_, items_, chat_) {}

void World::add_player(const PlayerData& p) {
    players_.kick_by_username(p.username);
    players_.add(p);
    restore_clan_membership(p);
    clans_.notify_login(p.entity_id, true);
}
void World::remove_player(uint16_t id) {
    clans_.notify_login(id, false);
    players_.remove(id);
}
void World::move_player(uint16_t id, uint16_t x, uint16_t y) { players_.move(id, x, y); }
void World::check_level_up(PlayerData& p) {
    uint8_t orig_level = p.level;
    while (true) {
        uint32_t limit = Equations::exp_required_for_level(p.level);
        if (p.exp < limit)
            break;
        p.level++;

        // Escalado aditivo: cada nivel suma un 10% del
        // HP/MP inicial, en vez de multiplicar el máximo por el nivel.
        uint16_t base_hp = GameConfig::get().initial_max_hp(p.race, p.cls);
        uint16_t base_mp = GameConfig::get().initial_max_mp(p.race, p.cls);
        uint16_t hp_per_level = static_cast<uint16_t>(base_hp * 0.10f);
        uint16_t mp_per_level = static_cast<uint16_t>(base_mp * 0.10f);

        p.max_hp = base_hp + static_cast<uint16_t>((p.level - 1)) * hp_per_level;

        if (!GameConfig::get().cls(p.cls).can_meditate) {
            p.max_mp = 0;
            p.mp = 0;
        } else {
            p.max_mp = base_mp + static_cast<uint16_t>((p.level - 1)) * mp_per_level;
        }
        p.hp = std::min(p.hp, p.max_hp);
        p.mp = std::min(p.mp, p.max_mp);
        if (p.level != orig_level)
            push_message(p.entity_id, 0, "¡Subiste al nivel " + std::to_string(p.level) + "!");
    }
}

void World::tp_player(uint16_t id, uint16_t x, uint16_t y) { players_.tp(id, x, y); }

const std::unordered_map<uint16_t, PlayerData>& World::get_players() const {
    return players_.all();
}
std::unordered_map<uint16_t, PlayerData>& World::get_players_mutable() {
    return players_.all_mutable();
}
const PlayerData* World::find_player(uint16_t id) const { return players_.find(id); }
PlayerData* World::get_player_mutable(uint16_t id) { return players_.find_mutable(id); }

SnapshotDTO World::build_snapshot(
        uint16_t client_id, uint32_t tick, const std::shared_ptr<std::vector<EntityDTO>>& entities,
        const std::shared_ptr<std::vector<SpellEventDTO>>& spell_events) const {
    return snapshot_.build(client_id, tick, entities, spell_events);
}
std::shared_ptr<std::vector<EntityDTO>> World::get_entities() const {
    return snapshot_.get_entities();
}

void World::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied) {
    collision_.update_occupied(pos, occupied);
}

void World::add_floor_item(uint8_t item_id, uint16_t x, uint16_t y, uint32_t gold,
                           uint32_t spawn_tick) {
    items_.add(item_id, x, y, gold, spawn_tick);
}
uint8_t World::pick_floor_item(uint16_t x, uint16_t y, uint32_t& gold_out) {
    return items_.pick(x, y, gold_out);
}
void World::cleanup_items(uint32_t current_tick) { items_.cleanup_expired(current_tick); }
void World::drop_player_loot(PlayerData& dead, uint32_t spawn_tick) {
    items_.drop_player_loot(dead);
    // sangre al morir el jugador
    items_.add(static_cast<uint8_t>(ItemId::BLOOD_STAIN), dead.pos_x, dead.pos_y, 0, spawn_tick);
}

void World::spawn_npc(NpcId type, uint16_t x, uint16_t y) { npcs_.spawn(type, x, y); }
void World::tick_npcs(uint32_t current_tick) { npcs_.tick(current_tick); }
NpcData* World::find_npc(uint16_t id) { return npcs_.find(id); }

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

void World::push_spell_event(uint16_t caster_id, uint8_t spell_id, uint16_t target_x,
                             uint16_t target_y, bool is_magic_projectile) {
    chat_.push_spell_event(caster_id, spell_id, target_x, target_y, is_magic_projectile);
}
std::shared_ptr<std::vector<SpellEventDTO>> World::get_spell_events() const {
    return chat_.get_spell_events();
}
void World::clear_spell_events() { chat_.clear_spell_events(); }

bool World::bank_deposit_item(uint16_t id, uint8_t slot) { return bank_.deposit_item(id, slot); }
bool World::bank_withdraw_item(uint16_t id, const std::string& n) {
    return bank_.withdraw_item(id, n);
}
bool World::bank_deposit_gold(uint16_t id, uint32_t amt) { return bank_.deposit_gold(id, amt); }
bool World::bank_withdraw_gold(uint16_t id, uint32_t amt) { return bank_.withdraw_gold(id, amt); }
std::string World::bank_list(uint16_t id) const { return bank_.list(id); }

bool World::clan_found(uint16_t f, const std::string& n) { return clans_.found(f, n); }
bool World::clan_join_request(uint16_t p, const std::string& n) {
    return clans_.join_request(p, n);
}
std::string World::clan_review(uint16_t f) const { return clans_.review(f); }
bool World::clan_accept(uint16_t f, const std::string& n) { return clans_.accept(f, n); }
bool World::clan_reject(uint16_t f, const std::string& n) { return clans_.reject(f, n); }
bool World::clan_ban(uint16_t f, const std::string& n) { return clans_.ban(f, n); }
bool World::clan_kick(uint16_t f, const std::string& n) { return clans_.kick(f, n); }
bool World::clan_leave(uint16_t p) { return clans_.leave(p); }
bool World::same_clan(uint16_t a, uint16_t b) const { return clans_.same_clan(a, b); }
void World::restore_clan_membership(const PlayerData& p) {
    if (p.has_clan()) {
        clans_.restore_membership(p.entity_id, std::string(p.clan_name), p.is_clan_founder);
    }
}
void World::clan_notify_login(uint16_t p, bool online) { clans_.notify_login(p, online); }
void World::clan_notify_attack(uint16_t a) { clans_.notify_attack(a); }

const std::vector<NpcData>& World::get_npcs() const { return npcs_.all(); }

WorldSpawner& World::spawner() { return spawner_; }

Mazmorra& World::add_dungeon(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    mazmorras_.emplace_back(npcs_, items_, x1, y1, x2, y2);
    return mazmorras_.back();
}

Mazmorra* World::get_dungeon_at(uint16_t x, uint16_t y) {
    for (Mazmorra& mazmorra: mazmorras_) {
        if (mazmorra.in_mazmorra(x, y))
            return &mazmorra;
    }
    return nullptr;
}

bool World::in_safe_zone(uint16_t x, uint16_t y) const { return spawner_.in_safe_zone(x, y); }

uint16_t World::find_player_by_name(const std::string& name) const {
    return players_.find_by_name(name);
}
int World::rand_range(int lo, int hi) {
    if (lo >= hi)
        return lo;
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}

void World::set_selected_npc(uint16_t client_id, NpcId type) {
    selected_service_npc_[client_id] = type;
}

NpcId World::get_selected_npc(uint16_t client_id) const {
    auto it = selected_service_npc_.find(client_id);
    return (it != selected_service_npc_.end()) ? it->second : NpcId::GOBLIN;  // GOBLIN = "ninguno"
}

void World::clear_selected_npc(uint16_t client_id) { selected_service_npc_.erase(client_id); }

bool World::player_near_service_npc(uint16_t client_id, NpcId required_type) const {
    const PlayerData* p = find_player(client_id);
    if (!p)
        return false;
    for (const auto& npc: get_npcs()) {
        if (npc.type != required_type)
            continue;
        int dist = std::abs(p->pos_x - npc.pos_x) + std::abs(p->pos_y - npc.pos_y);
        if (dist <= 2)
            return true;
    }
    return false;
}

bool World::find_nearby_priest_pos(uint16_t client_id, uint16_t& out_x, uint16_t& out_y) const {
    const PlayerData* p = find_player(client_id);
    if (!p)
        return false;
    for (const auto& npc: get_npcs()) {
        if (npc.type != NpcId::PRIEST)
            continue;
        int dist = std::abs(p->pos_x - npc.pos_x) + std::abs(p->pos_y - npc.pos_y);
        if (dist <= 2) {
            out_x = npc.pos_x;
            out_y = npc.pos_y;
            return true;
        }
    }
    return false;
}

uint8_t World::get_nearby_merchant_zone(uint16_t client_id) const {
    const PlayerData* p = find_player(client_id);
    if (!p)
        return 255;
    for (const auto& npc: get_npcs()) {
        if (npc.type != NpcId::MERCHANT)
            continue;
        int dist = std::abs(p->pos_x - npc.pos_x) + std::abs(p->pos_y - npc.pos_y);
        if (dist <= 2)
            return npc.zone_id;
    }
    return 255;
}

void World::spawn_npc_in_zone(NpcId type, uint16_t x, uint16_t y, uint8_t zone_id) {
    npcs_.spawn_with_zone(type, x, y, zone_id);
}
