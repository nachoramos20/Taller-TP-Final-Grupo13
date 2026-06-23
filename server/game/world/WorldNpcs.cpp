#include "WorldNpcs.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <string>
#include <utility>

#include "../../../common/protocol/protocol.h"
#include "../Items.h"

#include "WorldChat.h"
#include "WorldClans.h"
#include "WorldCollision.h"
#include "WorldItems.h"
#include "WorldPlayers.h"

int WorldNpcs::rand_range(int lo, int hi) {
    if (lo >= hi)
        return lo;
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}

void WorldNpcs::spawn_internal(NpcId type, uint16_t x, uint16_t y, uint8_t zone_id) {
    const NpcTemplate& tpl = Npcs::tpl(type);
    NpcData npc{};
    npc.entity_id = id_alloc.allocate();
    npc.type = type;
    npc.pos_x = x;
    npc.pos_y = y;
    npc.hp = tpl.max_hp;
    npc.max_hp = tpl.max_hp;
    npc.zone_id = zone_id;
    npcs.push_back(npc);
    collision.update(x, y, true);
}

void WorldNpcs::spawn(NpcId type, uint16_t x, uint16_t y) { spawn_internal(type, x, y, 255); }

void WorldNpcs::kill_all_in_zone(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    for (NpcData& n: npcs) {
        if (n.hp == 0)
            continue;
        if (n.pos_x >= x1 && n.pos_x <= x2 && n.pos_y >= y1 && n.pos_y <= y2) {
            n.hp = 0;
            collision.update(n.pos_x, n.pos_y, false);
        }
    }
    std::erase_if(npcs, [](const NpcData& n) { return n.hp == 0; });
}

NpcData* WorldNpcs::find(uint16_t id) {
    for (auto& n: npcs)
        if (n.entity_id == id)
            return &n;
    return nullptr;
}

void WorldNpcs::drop_npc_loot(const NpcData& npc) {
    const NpcTemplate& tpl = Npcs::tpl(npc.type);

    double roll = std::uniform_real_distribution<double>(0.0, 1.0)(rng);

    if (roll < 0.80) {
        return;
    } else if (roll < 0.88) {
        double factor = std::uniform_real_distribution<double>(0.01, 0.20)(rng);
        uint32_t gold = static_cast<uint32_t>(factor * tpl.max_hp);
        if (gold < 1)
            gold = 1;
        items.add(static_cast<uint8_t>(ItemId::GOLD_PILE), npc.pos_x, npc.pos_y, gold);
    } else if (roll < 0.89) {
        uint8_t potion = (rng() % 2 == 0) ? static_cast<uint8_t>(ItemId::HEALTH_POTION) :
                                            static_cast<uint8_t>(ItemId::MANA_POTION);
        items.add(potion, npc.pos_x, npc.pos_y, 0);
    } else {
        if (!tpl.drop_table.empty()) {
            uint8_t item = tpl.drop_table[rng() % tpl.drop_table.size()];
            items.add(item, npc.pos_x, npc.pos_y, 0);
        } else {
            static const std::vector<uint8_t> fallback = {
                    (uint8_t)ItemId::SWORD,         (uint8_t)ItemId::AXE,
                    (uint8_t)ItemId::HAMMER,        (uint8_t)ItemId::SIMPLE_BOW,
                    (uint8_t)ItemId::LEATHER_ARMOR, (uint8_t)ItemId::IRON_HELMET,
                    (uint8_t)ItemId::TURTLE_SHIELD,
            };
            items.add(fallback[rng() % fallback.size()], npc.pos_x, npc.pos_y, 0);
        }
    }
}

void WorldNpcs::cleanup_dead() {
    for (const auto& n: npcs) {
        if (n.hp == 0) {
            drop_npc_loot(n);
            items.add(static_cast<uint8_t>(ItemId::BLOOD_STAIN), n.pos_x, n.pos_y, 0, current_tick);
            collision.update(n.pos_x, n.pos_y, false);
            if (n.zone_id != 255 && n.zone_id < spawner.zones_mut().size()) {
                auto& z = spawner.zones_mut()[n.zone_id];
                if (z.alive_count > 0)
                    z.alive_count--;
            }
        }
    }
    npcs.erase(std::remove_if(npcs.begin(), npcs.end(), [](const NpcData& n) { return n.hp == 0; }),
               npcs.end());
}

void WorldNpcs::tick(uint32_t ct) {
    current_tick = ct;
    for (auto& npc: npcs) {
        if (npc.hp == 0)
            continue;
        const NpcTemplate& tpl = Npcs::tpl(npc.type);

        if (tpl.is_service)
            continue;

        PlayerData* nearest = nullptr;
        int best_dist = INT32_MAX;
        for (auto& [pid, player]: players.all_mutable()) {
            if (player.is_ghost)
                continue;
            if (spawner.in_safe_zone(player.pos_x, player.pos_y))
                continue;
            int dx = std::abs((int)npc.pos_x - (int)player.pos_x);
            int dy = std::abs((int)npc.pos_y - (int)player.pos_y);
            int dist = dx + dy;
            if (dist < best_dist) {
                best_dist = dist;
                nearest = &player;
            }
        }

        if (!nearest || best_dist > 15)
            continue;

        if (npc.attack_timer == 0 && best_dist <= (int)tpl.attack_range + 1) {
            // Probabilidad fija de esquivar (5%), no en función de la
            // agilidad: una fórmula basada en agilidad daba chances casi
            // imposibles de esquivar en la práctica.
            double dodge_roll = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
            bool dodged = dodge_roll < 0.05;

            if (dodged) {
                chat.push_message(nearest->entity_id, 1, "¡Esquivaste el ataque del NPC!");
            } else {
                int dmg = rand_range(tpl.dmg_min, tpl.dmg_max);

                // Solo la armadura equipada reduce el daño (la agilidad no
                // resta daño de NPCs, a diferencia del esquive).
                int armor_def = 0;
                auto add_armor = [&](uint8_t eq) {
                    if (eq != 0 && Items::exists(static_cast<ItemId>(eq))) {
                        const auto& it = Items::get(static_cast<ItemId>(eq));
                        armor_def += rand_range(it.min_value, it.max_value);
                    }
                };
                add_armor(nearest->equipped_armor);
                add_armor(nearest->equipped_helmet);
                add_armor(nearest->equipped_shield);
                dmg = std::max(1, dmg - armor_def);

                chat.push_message(nearest->entity_id, 1,
                                  "Recibiste " + std::to_string(dmg) + " de daño del " + tpl.name);

                nearest->meditating = false;

                if (nearest->cheat_infinite_hp) {
                    // Vida infinita: no recibe daño ni muere.
                } else if (nearest->hp <= (uint16_t)dmg) {
                    nearest->hp = 0;
                    nearest->is_ghost = true;
                    nearest->meditating = false;
                    collision.update(nearest->pos_x, nearest->pos_y, false);
                    items.drop_player_loot(*nearest);
                    items.add(static_cast<uint8_t>(ItemId::BLOOD_STAIN), nearest->pos_x,
                              nearest->pos_y, 0, current_tick);
                    chat.push_message(nearest->entity_id, 1, "Has muerto! Eres un fantasma.");
                    clans.notify_attack(nearest->entity_id);
                } else {
                    nearest->hp -= static_cast<uint16_t>(dmg);
                    clans.notify_attack(nearest->entity_id);
                }
            }
            npc.attack_timer = tpl.attack_cooldown;
        } else if (npc.attack_timer > 0) {
            npc.attack_timer--;
        }

        // No persigue dentro de una zona segura.
        if (npc.move_timer == 0 && best_dist > 1) {
            int dx = (int)nearest->pos_x - (int)npc.pos_x;
            int dy = (int)nearest->pos_y - (int)npc.pos_y;

            uint16_t nx = npc.pos_x, ny = npc.pos_y;
            if (std::abs(dx) >= std::abs(dy)) {
                nx = npc.pos_x + (dx > 0 ? 1 : -1);
            } else {
                ny = npc.pos_y + (dy > 0 ? 1 : -1);
            }

            bool can_move = collision.in_bounds(nx, ny) && !collision.is_occupied(nx, ny) &&
                            !spawner.in_safe_zone(nx, ny);

            if (can_move) {
                collision.update(npc.pos_x, npc.pos_y, false);
                npc.pos_x = nx;
                npc.pos_y = ny;
                collision.update(nx, ny, true);
                if (dx > 0)
                    npc.direction = static_cast<uint8_t>(MoveDirection::EAST);
                else if (dx < 0)
                    npc.direction = static_cast<uint8_t>(MoveDirection::WEST);
                else if (dy > 0)
                    npc.direction = static_cast<uint8_t>(MoveDirection::SOUTH);
                else
                    npc.direction = static_cast<uint8_t>(MoveDirection::NORTH);
            }
            npc.move_timer = tpl.move_cooldown;
        } else if (npc.move_timer > 0) {
            npc.move_timer--;
        }
    }

    cleanup_dead();

    auto pending = spawner.tick(static_cast<uint16_t>(npcs.size()));
    for (const auto& s: pending) {
        spawn_internal(s.type, s.x, s.y, s.zone_id);
    }
}

void WorldNpcs::spawn_with_zone(NpcId type, uint16_t x, uint16_t y, uint8_t zone_id) {
    spawn_internal(type, x, y, zone_id);
}
