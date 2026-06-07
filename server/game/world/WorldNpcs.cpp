#include "WorldNpcs.h"
#include "WorldCollision.h"
#include "WorldPlayers.h"
#include "WorldItems.h"
#include "WorldChat.h"
#include "WorldClans.h"
#include "../Items.h"
#include "../../../common/protocol/protocol.h"

#include <algorithm>
#include <cmath>
#include <climits>
#include <cstdlib>
#include <string>

int WorldNpcs::rand_range(int lo, int hi) {
    if (lo >= hi) return lo;
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}

void WorldNpcs::spawn(NpcId type, uint16_t x, uint16_t y) {
    const NpcTemplate& tpl = Npcs::tpl(type);
    NpcData npc{};
    npc.entity_id    = id_alloc.allocate();
    npc.type         = type;
    npc.pos_x        = x;
    npc.pos_y        = y;
    npc.hp           = tpl.max_hp;
    npc.max_hp       = tpl.max_hp;
    npc.move_timer   = 0;
    npc.attack_timer = 0;
    npcs.push_back(npc);
    collision.update(x, y, true);
}

NpcData* WorldNpcs::find(uint16_t id) {
    for (auto& n : npcs)
        if (n.entity_id == id) return &n;
    return nullptr;
}

void WorldNpcs::tick() {
    for (auto& npc : npcs) {
        const NpcTemplate& tpl = Npcs::tpl(npc.type);

        // Jugador vivo más cercano
        PlayerData* nearest = nullptr;
        int best_dist = INT32_MAX;
        for (auto& [pid, player] : players.all_mutable()) {
            if (player.is_ghost) continue;
            int dx = std::abs((int)npc.pos_x - (int)player.pos_x);
            int dy = std::abs((int)npc.pos_y - (int)player.pos_y);
            int dist = dx + dy;
            if (dist < best_dist) { best_dist = dist; nearest = &player; }
        }

        if (!nearest || best_dist > 15) continue;

        // Ataque
        if (npc.attack_timer == 0 && best_dist <= (int)tpl.attack_range + 1) {
            double dodge_roll = std::pow(
                std::uniform_real_distribution<double>(0.0, 1.0)(rng),
                nearest->agility);
            bool dodged = dodge_roll < 0.001;

            if (dodged) {
                chat.push_message(nearest->entity_id, 1, "Esquivaste el ataque del NPC!");
            } else {
                int dmg = rand_range(tpl.dmg_min, tpl.dmg_max);
                int def = rand_range(0, nearest->agility / 4);
                dmg = std::max(1, dmg - def);

                int armor_def = 0;
                if (nearest->equipped_armor != 0 &&
                        Items::exists(static_cast<ItemId>(nearest->equipped_armor))) {
                    const auto& a = Items::get(static_cast<ItemId>(nearest->equipped_armor));
                    armor_def += rand_range(a.min_value, a.max_value);
                }
                if (nearest->equipped_helmet != 0 &&
                        Items::exists(static_cast<ItemId>(nearest->equipped_helmet))) {
                    const auto& h = Items::get(static_cast<ItemId>(nearest->equipped_helmet));
                    armor_def += rand_range(h.min_value, h.max_value);
                }
                if (nearest->equipped_shield != 0 &&
                        Items::exists(static_cast<ItemId>(nearest->equipped_shield))) {
                    const auto& s = Items::get(static_cast<ItemId>(nearest->equipped_shield));
                    armor_def += rand_range(s.min_value, s.max_value);
                }
                dmg = std::max(1, dmg - armor_def);

                chat.push_message(nearest->entity_id, 1,
                    "Recibiste " + std::to_string(dmg) + " de daño del " + tpl.name);

                if (nearest->hp <= (uint16_t)dmg) {
                    nearest->hp = 0;
                    nearest->is_ghost = true;
                    nearest->meditating = false;
                    collision.update(nearest->pos_x, nearest->pos_y, false);
                    items.drop_player_loot(*nearest);
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

            if (collision.in_bounds(nx, ny) && !collision.is_occupied(nx, ny)) {
                collision.update(npc.pos_x, npc.pos_y, false);
                npc.pos_x = nx;
                npc.pos_y = ny;
                collision.update(nx, ny, true);
                if (dx > 0)      npc.direction = static_cast<uint8_t>(MoveDirection::EAST);
                else if (dx < 0) npc.direction = static_cast<uint8_t>(MoveDirection::WEST);
                else if (dy > 0) npc.direction = static_cast<uint8_t>(MoveDirection::SOUTH);
                else             npc.direction = static_cast<uint8_t>(MoveDirection::NORTH);
            }
            npc.move_timer = tpl.move_cooldown;
        } else if (npc.move_timer > 0) {
            npc.move_timer--;
        }
    }
}
