#include "World.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <utility>

#include "../../common/protocol/protocol.h"
#include "Items.h"

World::World(uint16_t width, uint16_t height)
    : width(width), height(height), rng(std::random_device{}()) {
    for (uint16_t x = 0; x < width; ++x)
        for (uint16_t y = 0; y < height; ++y)
            occupied_positions[{x, y}] = false;
}

// Players

void World::add_player(const PlayerData& player_data) {
    players_map.emplace(player_data.entity_id, player_data);
    update_occupied({player_data.pos_x, player_data.pos_y}, true);
    clan_notify_login(player_data.entity_id, true);
}

void World::remove_player(uint16_t client_id) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;
    clan_notify_login(client_id, false);
    update_occupied({it->second.pos_x, it->second.pos_y}, false);
    players_map.erase(it);
}

void World::set_direction_from_delta(PlayerData& player, int dx, int dy) {
    if (dx == 1)       player.direction = static_cast<uint8_t>(MoveDirection::EAST);
    else if (dx == -1) player.direction = static_cast<uint8_t>(MoveDirection::WEST);
    else if (dy == 1)  player.direction = static_cast<uint8_t>(MoveDirection::SOUTH);
    else if (dy == -1) player.direction = static_cast<uint8_t>(MoveDirection::NORTH);
}

void World::move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y) {
    auto it = players_map.find(client_id);
    if (it == players_map.end()) return;
    PlayerData& player = it->second;

    if (new_x >= width || new_y >= height) return;

    const int dx = static_cast<int>(new_x) - static_cast<int>(player.pos_x);
    const int dy = static_cast<int>(new_y) - static_cast<int>(player.pos_y);
    if (std::abs(dx) + std::abs(dy) != 1) return;

    set_direction_from_delta(player, dx, dy);

    // Fantasmas pueden moverse pero colisionan igual
    const std::pair<uint16_t, uint16_t> new_pos{new_x, new_y};
    if (occupied_positions[new_pos]) return;

    occupied_positions[{player.pos_x, player.pos_y}] = false;
    occupied_positions[new_pos] = true;

    player.pos_x = new_x;
    player.pos_y = new_y;

    // Cualquier movimiento cancela meditación
    player.meditating = false;
}

const std::unordered_map<uint16_t, PlayerData>& World::get_players() const {
    return players_map;
}

std::unordered_map<uint16_t, PlayerData>& World::get_players_mutable() {
    return players_map;
}

const PlayerData* World::find_player(uint16_t client_id) const {
    auto it = players_map.find(client_id);
    return it == players_map.end() ? nullptr : &it->second;
}

PlayerData* World::get_player_mutable(uint16_t client_id) {
    auto it = players_map.find(client_id);
    return it == players_map.end() ? nullptr : &it->second;
}

// Snapshot

std::shared_ptr<std::vector<EntityDTO>> World::get_entities() const {
    auto entities = std::make_shared<std::vector<EntityDTO>>();
    entities->reserve(players_map.size() + npcs.size() + floor_items.size());

    // Jugadores
    for (const auto& [id, player] : players_map) {
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

    // NPCs
    for (const auto& npc : npcs) {
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

    // Items en el suelo
    for (const auto& fi : floor_items) {
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

SnapshotDTO World::build_snapshot(uint16_t client_id,
                                   uint32_t tick,
                                   const std::shared_ptr<std::vector<EntityDTO>>& entities) const {
    SnapshotDTO snap{};
    snap.tick = tick;

    if (const PlayerData* p = find_player(client_id)) {
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

        for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; ++i)
            snap.inventory[i] = p->inventory[i];

        snap.equipped_wpn  = p->equipped_weapon;
        snap.equipped_arm  = p->equipped_armor;
        snap.equipped_helm = p->equipped_helmet;
        snap.equipped_shld = p->equipped_shield;
    }

    snap.entities = entities;
    snap.messages = const_cast<World*>(this)->collect_messages(client_id);
    return snap;
}

// Colisiones

void World::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied) {
    if (pos.first >= width || pos.second >= height) return;
    occupied_positions[pos] = occupied;
}

void World::revisar_colisiones() {
    for (auto& [id, player] : players_map)
        update_occupied({player.pos_x, player.pos_y}, true);
    for (auto& npc : npcs)
        update_occupied({npc.pos_x, npc.pos_y}, true);
}

// Floor items

void World::add_floor_item(uint8_t item_id, uint16_t x, uint16_t y, uint32_t gold) {
    FloorItem fi{};
    fi.entity_id  = new_entity_id();
    fi.item_id    = item_id;
    fi.pos_x      = x;
    fi.pos_y      = y;
    fi.gold_amount = gold;
    floor_items.push_back(fi);
}

uint8_t World::pick_floor_item(uint16_t x, uint16_t y, uint32_t& gold_out) {
    gold_out = 0;
    for (auto it = floor_items.begin(); it != floor_items.end(); ++it) {
        if (it->pos_x == x && it->pos_y == y) {
            uint8_t  id   = it->item_id;
            gold_out      = it->gold_amount;
            floor_items.erase(it);
            return id;
        }
    }
    return 0;
}

void World::drop_player_loot(PlayerData& dead) {
    // Oro en exceso (enunciado: OroMax = 100 * nivel^1.1)
    uint32_t oro_max = static_cast<uint32_t>(
        100.0 * std::pow(static_cast<double>(dead.level), 1.1));
    if (dead.gold > oro_max) {
        uint32_t excess = dead.gold - oro_max;
        dead.gold = oro_max;
        add_floor_item(static_cast<uint8_t>(ItemId::GOLD_PILE), dead.pos_x, dead.pos_y, excess);
    }
    // Inventario completo cae al piso
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (dead.inventory[i] != 0) {
            add_floor_item(dead.inventory[i], dead.pos_x, dead.pos_y, 0);
            dead.inventory[i] = 0;
        }
    }
}

// NPCs

void World::spawn_npc(NpcId type, uint16_t x, uint16_t y) {
    const NpcTemplate& tpl = Npcs::tpl(type);
    NpcData npc{};
    npc.entity_id    = new_entity_id();
    npc.type         = type;
    npc.pos_x        = x;
    npc.pos_y        = y;
    npc.hp           = tpl.max_hp;
    npc.max_hp       = tpl.max_hp;
    npc.move_timer   = 0;
    npc.attack_timer = 0;
    npcs.push_back(npc);
    update_occupied({x, y}, true);
}

NpcData* World::find_npc(uint16_t id) {
    for (auto& n : npcs)
        if (n.entity_id == id) return &n;
    return nullptr;
}

void World::tick_npcs() {
    std::vector<uint16_t> dead_npcs;

    for (auto& npc : npcs) {
        const NpcTemplate& tpl = Npcs::tpl(npc.type);

        // Buscar jugador vivo más cercano en rango
        PlayerData* nearest = nullptr;
        int best_dist = INT32_MAX;
        for (auto& [pid, player] : players_map) {
            if (player.is_ghost) continue;
            int dx = std::abs((int)npc.pos_x - (int)player.pos_x);
            int dy = std::abs((int)npc.pos_y - (int)player.pos_y);
            int dist = dx + dy;
            if (dist < best_dist) {
                best_dist = dist;
                nearest = &player;
            }
        }

        if (!nearest || best_dist > 15) continue; // nadie cerca

        // Ataque
        if (npc.attack_timer == 0 && best_dist <= (int)tpl.attack_range + 1) {
            // Esquive del jugador
            double dodge_roll = std::pow(
                std::uniform_real_distribution<double>(0.0, 1.0)(rng),
                nearest->agility);
            bool dodged = dodge_roll < 0.001;

            if (dodged) {
                push_message(nearest->entity_id, 1, "Esquivaste el ataque del NPC!");
            } else {
                int dmg = rand_range(tpl.dmg_min, tpl.dmg_max);
                int def = rand_range(0, nearest->agility / 4);
                dmg = std::max(1, dmg - def);

                // Defensa por equipo
                int armor_def = 0;
                if (nearest->equipped_armor != 0 && Items::exists(static_cast<ItemId>(nearest->equipped_armor))) {
                    const auto& a = Items::get(static_cast<ItemId>(nearest->equipped_armor));
                    armor_def += rand_range(a.min_value, a.max_value);
                }
                if (nearest->equipped_helmet != 0 && Items::exists(static_cast<ItemId>(nearest->equipped_helmet))) {
                    const auto& h = Items::get(static_cast<ItemId>(nearest->equipped_helmet));
                    armor_def += rand_range(h.min_value, h.max_value);
                }
                if (nearest->equipped_shield != 0 && Items::exists(static_cast<ItemId>(nearest->equipped_shield))) {
                    const auto& s = Items::get(static_cast<ItemId>(nearest->equipped_shield));
                    armor_def += rand_range(s.min_value, s.max_value);
                }
                dmg = std::max(1, dmg - armor_def);

                std::string msg = "Recibiste " + std::to_string(dmg) + " de daño del " + tpl.name;
                push_message(nearest->entity_id, 1, msg);

                if (nearest->hp <= (uint16_t)dmg) {
                    nearest->hp = 0;
                    nearest->is_ghost = true;
                    nearest->meditating = false;
                    update_occupied({nearest->pos_x, nearest->pos_y}, false);
                    drop_player_loot(*nearest);
                    push_message(nearest->entity_id, 1, "Has muerto! Eres un fantasma.");
                    clan_notify_attack(nearest->entity_id); // en realidad murió, pero notificamos
                } else {
                    nearest->hp -= static_cast<uint16_t>(dmg);
                    clan_notify_attack(nearest->entity_id);
                }
            }
            npc.attack_timer = tpl.attack_cooldown;
        } else if (npc.attack_timer > 0) {
            npc.attack_timer--;
        }

        // Movimiento hacia el jugador
        if (npc.move_timer == 0 && best_dist > 1) {
            int dx = (int)nearest->pos_x - (int)npc.pos_x;
            int dy = (int)nearest->pos_y - (int)npc.pos_y;

            uint16_t nx = npc.pos_x, ny = npc.pos_y;
            if (std::abs(dx) >= std::abs(dy)) {
                nx = npc.pos_x + (dx > 0 ? 1 : -1);
            } else {
                ny = npc.pos_y + (dy > 0 ? 1 : -1);
            }

            if (nx < width && ny < height && !occupied_positions[{nx, ny}]) {
                update_occupied({npc.pos_x, npc.pos_y}, false);
                npc.pos_x = nx;
                npc.pos_y = ny;
                update_occupied({nx, ny}, true);
                // dirección
                int mx = (int)nx - (int)(npc.pos_x > 0 ? npc.pos_x : 0);
                int my = (int)ny - (int)(npc.pos_y > 0 ? npc.pos_y : 0);
                if (dx > 0) npc.direction = static_cast<uint8_t>(MoveDirection::EAST);
                else if (dx < 0) npc.direction = static_cast<uint8_t>(MoveDirection::WEST);
                else if (dy > 0) npc.direction = static_cast<uint8_t>(MoveDirection::SOUTH);
                else npc.direction = static_cast<uint8_t>(MoveDirection::NORTH);
                (void)mx; (void)my;
            }
            npc.move_timer = tpl.move_cooldown;
        } else if (npc.move_timer > 0) {
            npc.move_timer--;
        }
    }
}

// Mensajes / Chat

void World::push_message(uint16_t to_id, uint8_t type, const std::string& text) {
    pending_messages.push_back({to_id, type, text});
}

void World::push_broadcast(uint8_t type, const std::string& text) {
    pending_messages.push_back({0, type, text});
}

std::shared_ptr<std::vector<ChatMessageDTO>> World::collect_messages(uint16_t client_id) {
    auto msgs = std::make_shared<std::vector<ChatMessageDTO>>();
    for (auto it = pending_messages.begin(); it != pending_messages.end(); ) {
        if (it->to_client_id == 0 || it->to_client_id == client_id) {
            ChatMessageDTO m{};
            m.msg_type = it->msg_type;
            m.text     = it->text;
            msgs->push_back(m);
            if (it->to_client_id != 0) {
                it = pending_messages.erase(it);
                continue;
            }
        }
        ++it;
    }
    // Limpiar broadcast después de pasar por todos (llamado una vez por player → acumulamos
    // y limpiamos los broadcast al final del tick desde ServerGameLoop)
    return msgs;
}

void World::clear_broadcast_messages() {
    pending_messages.erase(
        std::remove_if(pending_messages.begin(), pending_messages.end(),
            [](const PendingMessage& m){ return m.to_client_id == 0; }),
        pending_messages.end());
}

// Banco

bool World::bank_deposit_item(uint16_t client_id, uint8_t inv_slot) {
    PlayerData* p = get_player_mutable(client_id);
    if (!p || p->is_ghost || inv_slot >= PlayerData::INVENTORY_SIZE) return false;

    uint8_t item = p->inventory[inv_slot];
    if (item == 0) return false;

    p->inventory[inv_slot] = 0;
    bank_inventories[p->username].push_back(item);
    push_message(client_id, 0, "Depositaste el objeto en el banco.");
    return true;
}

bool World::bank_withdraw_item(uint16_t client_id, const std::string& item_name) {
    PlayerData* p = get_player_mutable(client_id);
    if (!p || p->is_ghost) return false;

    // Buscar slot libre
    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        push_message(client_id, 0, "Inventario lleno.");
        return false;
    }

    auto& inv = bank_inventories[p->username];
    for (auto it = inv.begin(); it != inv.end(); ++it) {
        if (Items::exists(static_cast<ItemId>(*it))) {
            const auto& def = Items::get(static_cast<ItemId>(*it));
            if (def.name == item_name) {
                p->inventory[free_slot] = *it;
                inv.erase(it);
                push_message(client_id, 0, "Retiraste " + item_name + " del banco.");
                return true;
            }
        }
    }
    push_message(client_id, 0, "No encontraste ese objeto en el banco.");
    return false;
}

bool World::bank_deposit_gold(uint16_t client_id, uint32_t amount) {
    PlayerData* p = get_player_mutable(client_id);
    if (!p || p->is_ghost || amount == 0 || p->gold < amount) return false;

    p->gold -= amount;
    bank_gold[p->username] += amount;
    push_message(client_id, 0, "Depositaste " + std::to_string(amount) + " de oro en el banco.");
    return true;
}

bool World::bank_withdraw_gold(uint16_t client_id, uint32_t amount) {
    PlayerData* p = get_player_mutable(client_id);
    if (!p || p->is_ghost || amount == 0) return false;

    auto it = bank_gold.find(p->username);
    if (it == bank_gold.end() || it->second < amount) {
        push_message(client_id, 0, "No tenés suficiente oro en el banco.");
        return false;
    }
    it->second -= amount;
    p->gold += amount;
    push_message(client_id, 0, "Retiraste " + std::to_string(amount) + " de oro del banco.");
    return true;
}

std::string World::bank_list(uint16_t client_id) const {
    const PlayerData* p = find_player(client_id);
    if (!p) return "";

    std::ostringstream oss;
    oss << "=== Banco de " << p->username << " ===\n";

    auto git = bank_gold.find(p->username);
    oss << "Oro: " << (git != bank_gold.end() ? git->second : 0) << "\n";

    auto iit = bank_inventories.find(p->username);
    if (iit != bank_inventories.end()) {
        for (uint8_t raw : iit->second) {
            if (Items::exists(static_cast<ItemId>(raw)))
                oss << "- " << Items::get(static_cast<ItemId>(raw)).name << "\n";
        }
    }
    return oss.str();
}

// Clanes

bool World::clan_found(uint16_t founder_id, const std::string& clan_name) {
    const PlayerData* p = find_player(founder_id);
    if (!p) return false;

    // Ya está en un clan
    if (player_clan.count(founder_id)) {
        push_message(founder_id, 0, "Ya perteneces a un clan.");
        return false;
    }
    // Nivel mínimo 6
    if (p->level < 6) {
        push_message(founder_id, 0, "Necesitas nivel 6 para fundar un clan.");
        return false;
    }
    // Nombre único
    if (clans_by_name.count(clan_name)) {
        push_message(founder_id, 0, "Ya existe un clan con ese nombre.");
        return false;
    }

    Clan clan;
    clan.name       = clan_name;
    clan.founder_id = founder_id;
    clan.members.push_back(founder_id);
    clans_by_name[clan_name] = clan;
    player_clan[founder_id] = clan_name;

    push_message(founder_id, 0, "Fundaste el clan \"" + clan_name + "\"!");
    return true;
}

bool World::clan_join_request(uint16_t player_id, const std::string& clan_name) {
    auto it = clans_by_name.find(clan_name);
    if (it == clans_by_name.end()) {
        push_message(player_id, 0, "No existe el clan \"" + clan_name + "\".");
        return false;
    }
    Clan& clan = it->second;

    if (player_clan.count(player_id)) {
        push_message(player_id, 0, "Ya perteneces a un clan.");
        return false;
    }
    if (clan.is_banned(player_id)) {
        push_message(player_id, 0, "Fuiste baneado de ese clan.");
        return false;
    }
    if (clan.is_pending(player_id)) {
        push_message(player_id, 0, "Ya enviaste un pedido.");
        return false;
    }
    if ((int)clan.members.size() >= Clan::MAX_MEMBERS) {
        push_message(player_id, 0, "El clan está lleno.");
        return false;
    }

    clan.pending.push_back(player_id);
    push_message(player_id, 0, "Pedido enviado al clan \"" + clan_name + "\".");
    push_message(clan.founder_id, 0, "Nuevo pedido de ingreso al clan de " +
        std::string(find_player(player_id) ? find_player(player_id)->username : "?") + ".");
    return true;
}

std::string World::clan_review(uint16_t founder_id) const {
    for (const auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        std::ostringstream oss;
        oss << "=== Clan \"" << name << "\" ===\nMiembros:\n";
        for (uint16_t mid : clan.members) {
            const PlayerData* mp = find_player(mid);
            oss << "  - " << (mp ? mp->username : std::to_string(mid)) << "\n";
        }
        oss << "Pedidos pendientes:\n";
        for (uint16_t pid : clan.pending) {
            const PlayerData* pp = find_player(pid);
            oss << "  - " << (pp ? pp->username : std::to_string(pid)) << "\n";
        }
        return oss.str();
    }
    return "No eres fundador de ningún clan.";
}

bool World::clan_accept(uint16_t founder_id, const std::string& nick) {
    uint16_t target = find_player_by_name(nick);
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        if (!clan.is_pending(target)) {
            push_message(founder_id, 0, nick + " no tiene pedido pendiente.");
            return false;
        }
        clan.remove_pending(target);
        clan.members.push_back(target);
        player_clan[target] = name;
        push_message(target, 0, "¡Fuiste aceptado en el clan \"" + name + "\"!");
        push_message(founder_id, 0, nick + " fue aceptado en el clan.");
        return true;
    }
    return false;
}

bool World::clan_reject(uint16_t founder_id, const std::string& nick) {
    uint16_t target = find_player_by_name(nick);
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        clan.remove_pending(target);
        push_message(target, 0, "Tu pedido al clan \"" + name + "\" fue rechazado.");
        push_message(founder_id, 0, "Rechazaste a " + nick + ".");
        return true;
    }
    return false;
}

bool World::clan_ban(uint16_t founder_id, const std::string& nick) {
    uint16_t target = find_player_by_name(nick);
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        clan.remove_pending(target);
        clan.banned.insert(target);
        push_message(target, 0, "Fuiste baneado del clan \"" + name + "\".");
        push_message(founder_id, 0, "Baneaste a " + nick + " del clan.");
        return true;
    }
    return false;
}

bool World::clan_kick(uint16_t founder_id, const std::string& nick) {
    uint16_t target = find_player_by_name(nick);
    if (target == 0) return false;
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        if (!clan.is_member(target)) {
            push_message(founder_id, 0, nick + " no es miembro del clan.");
            return false;
        }
        if (target == founder_id) {
            push_message(founder_id, 0, "El fundador no puede abandonar el clan.");
            return false;
        }
        clan.remove_member(target);
        player_clan.erase(target);
        push_message(target, 0, "Fuiste expulsado del clan \"" + name + "\".");
        push_message(founder_id, 0, "Expulsaste a " + nick + " del clan.");
        return true;
    }
    return false;
}

bool World::clan_leave(uint16_t player_id) {
    auto it = player_clan.find(player_id);
    if (it == player_clan.end()) {
        push_message(player_id, 0, "No perteneces a ningún clan.");
        return false;
    }
    Clan& clan = clans_by_name[it->second];
    if (clan.founder_id == player_id) {
        push_message(player_id, 0, "El fundador no puede dejar el clan.");
        return false;
    }
    clan.remove_member(player_id);
    player_clan.erase(player_id);
    push_message(player_id, 0, "Abandonaste el clan.");
    return true;
}

bool World::same_clan(uint16_t a, uint16_t b) const {
    auto ia = player_clan.find(a);
    auto ib = player_clan.find(b);
    if (ia == player_clan.end() || ib == player_clan.end()) return false;
    return ia->second == ib->second;
}

void World::clan_notify_login(uint16_t player_id, bool online) {
    auto it = player_clan.find(player_id);
    if (it == player_clan.end()) return;

    const Clan& clan = clans_by_name[it->second];
    const PlayerData* p = find_player(player_id);
    std::string name = p ? std::string(p->username) : std::to_string(player_id);
    std::string msg = "[Clan] " + name + (online ? " entró al juego." : " salió del juego.");
    for (uint16_t mid : clan.members)
        if (mid != player_id)
            push_message(mid, 0, msg);
}

void World::clan_notify_attack(uint16_t attacked_id) {
    auto it = player_clan.find(attacked_id);
    if (it == player_clan.end()) return;

    const Clan& clan = clans_by_name[it->second];
    const PlayerData* p = find_player(attacked_id);
    std::string name = p ? std::string(p->username) : std::to_string(attacked_id);
    std::string msg = "[Clan] " + name + " está siendo atacado!";
    for (uint16_t mid : clan.members)
        if (mid != attacked_id)
            push_message(mid, 0, msg);
}

// Helpers

uint16_t World::find_player_by_name(const std::string& name) const {
    for (const auto& [id, p] : players_map)
        if (std::string(p.username) == name) return id;
    return 0;
}

int World::rand_range(int lo, int hi) {
    if (lo >= hi) return lo;
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}
