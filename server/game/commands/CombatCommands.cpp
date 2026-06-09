#include "Commands.h"
#include "../Items.h"
#include "../Stats.h"
#include <algorithm>
#include <cmath>
#include <random>

// RNG local

static std::mt19937& local_rng() {
    static std::mt19937 r(std::random_device{}());
    return r;
}

static int rand_range(int lo, int hi) {
    if (lo >= hi) return lo;
    return std::uniform_int_distribution<int>(lo, hi)(local_rng());
}

// Helpers de geometría y fair-play

static int manhattan(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by) {
    return std::abs((int)ax - (int)bx) + std::abs((int)ay - (int)by);
}

static bool fair_play_ok(const PlayerData& a, const PlayerData& b) {
    if (a.level <= 12 || b.level <= 12) return false;
    return std::abs((int)a.level - (int)b.level) <= 10;
}

// Helpers de combate

// Estructura que describe el arma actualmente equipada.
// Se usa para centralizar la lectura del slot de arma.
struct WeaponInfo {
    ItemKind  kind      = ItemKind::NONE;
    uint16_t  mana_cost = 0;
    int       range     = 1;
    int       dmin      = 1;
    int       dmax      = 3;
    bool      valid     = false; // false → "a puño limpio"
};

static WeaponInfo read_weapon(const PlayerData& p) {
    WeaponInfo w;
    uint8_t slot = p.equipped_weapon;
    if (slot == 0xFF || slot >= PlayerData::INVENTORY_SIZE) return w;

    uint8_t item_id = p.inventory[slot];
    if (item_id == 0 || !Items::exists(static_cast<ItemId>(item_id))) return w;

    const ItemDef& def = Items::get(static_cast<ItemId>(item_id));
    w.kind      = def.kind;
    w.mana_cost = def.mana_cost;
    w.range     = (def.range_tiles > 0) ? def.range_tiles : 1;
    w.dmin      = def.min_value;
    w.dmax      = def.max_value;
    w.valid     = true;
    return w;
}

static uint16_t calc_weapon_damage(const PlayerData& attacker, const WeaponInfo& w) {
    int raw = attacker.strength * rand_range(w.dmin, w.dmax);
    return static_cast<uint16_t>(raw);
}

static uint16_t calc_defense(const PlayerData& defender) {
    int def = 0;

    auto add_piece = [&](uint8_t slot) {
        if (slot == 0xFF || slot >= PlayerData::INVENTORY_SIZE) return;
        uint8_t item_id = defender.inventory[slot];
        if (item_id == 0 || !Items::exists(static_cast<ItemId>(item_id))) return;
        const auto& d = Items::get(static_cast<ItemId>(item_id));
        def += rand_range(d.min_value, d.max_value);
    };

    add_piece(defender.equipped_armor);
    add_piece(defender.equipped_helmet);
    add_piece(defender.equipped_shield);
    return static_cast<uint16_t>(def);
}

static bool try_dodge(uint16_t agility) {
    double r = std::uniform_real_distribution<double>(0.0, 1.0)(local_rng());
    return std::pow(r, agility) < 0.001;
}

static bool is_critical() {
    return std::uniform_real_distribution<double>(0.0, 1.0)(local_rng()) < 0.05;
}

static uint32_t exp_per_damage(uint16_t damage, uint8_t my_level, uint8_t other_level) {
    int factor = std::max((int)other_level - (int)my_level + 10, 0);
    return static_cast<uint32_t>(damage) * static_cast<uint32_t>(factor);
}

static uint32_t exp_on_kill(uint16_t other_max_hp, uint8_t my_level, uint8_t other_level) {
    double r = std::uniform_real_distribution<double>(0.0, 0.1)(local_rng());
    int factor = std::max((int)other_level - (int)my_level + 10, 0);
    return static_cast<uint32_t>(r * other_max_hp * factor);
}

static uint32_t gold_drop_npc(uint16_t npc_max_hp) {
    double r = std::uniform_real_distribution<double>(0.0, 0.2)(local_rng());
    return static_cast<uint32_t>(r * npc_max_hp);
}

static void check_level_up(PlayerData& p, World& world) {
    uint8_t orig_level = p.level;
    while (true) {
        uint32_t limit = static_cast<uint32_t>(1000.0 * std::pow((double)p.level, 1.8));
        if (p.exp < limit) break;
        p.level++;
        p.max_hp = Stats::initial_max_hp(p.race, p.cls) * p.level;
        if (static_cast<Class>(p.cls) == Class::WARRIOR) {
            p.max_mp = 0;
            p.mp     = 0;
        } else {
            p.max_mp = Stats::initial_max_mp(p.race, p.cls) * p.level;
        }
        p.hp = std::min(p.hp, p.max_hp);
        p.mp = std::min(p.mp, p.max_mp);
        if (p.level != orig_level)
            world.push_message(p.entity_id, 0, "¡Subiste al nivel " + std::to_string(p.level) + "!");
    }
}

static void npc_drop(const NpcData& npc, const NpcTemplate& tpl, World& world) {
    double r = std::uniform_real_distribution<double>(0.0, 1.0)(local_rng());
    if (r < 0.80) return;
    if (r < 0.88) {
        double gr = std::uniform_real_distribution<double>(0.01, 0.2)(local_rng());
        uint32_t gold = static_cast<uint32_t>(gr * tpl.max_hp);
        world.add_floor_item(static_cast<uint8_t>(ItemId::GOLD_PILE), npc.pos_x, npc.pos_y, gold);
        return;
    }
    if (r < 0.89) {
        uint8_t pot = (rand_range(0, 1) == 0)
            ? static_cast<uint8_t>(ItemId::HEALTH_POTION)
            : static_cast<uint8_t>(ItemId::MANA_POTION);
        world.add_floor_item(pot, npc.pos_x, npc.pos_y, 0);
        return;
    }
    if (r < 0.90) {
        if (!tpl.drop_table.empty()) {
            int idx = rand_range(0, (int)tpl.drop_table.size() - 1);
            world.add_floor_item(tpl.drop_table[idx], npc.pos_x, npc.pos_y, 0);
        }
    }
}

// AttackCommand (PvP)

AttackCommand::AttackCommand(uint16_t c, uint16_t t)
    : client_id(c), target_id(t) {}

void AttackCommand::execute(World& world) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    PlayerData* target   = world.get_player_mutable(target_id);

    if (!attacker) return;

    if (!target) {
        if (world.find_npc(target_id)) {
            AttackNpcCommand npc_cmd(client_id, target_id);
            npc_cmd.execute(world);
        }
        return;
    }

    if (attacker->is_ghost || target->is_ghost) return;
    if (attacker->attack_cooldown > 0) return;

    if (!fair_play_ok(*attacker, *target)) {
        world.push_message(client_id, 0, "No puedes atacar a ese jugador (fair-play).");
        return;
    }
    if (world.same_clan(client_id, target_id)) {
        world.push_message(client_id, 0, "No puedes atacar a un compañero de clan.");
        return;
    }

    attacker->meditating = false;

    WeaponInfo w = read_weapon(*attacker);

    // Verificar mana para armas mágicas
    if (w.kind == ItemKind::WEAPON_MAGIC) {
        if (attacker->mp < w.mana_cost) {
            world.push_message(client_id, 0,
                "No tenés maná suficiente para disparar el arma mágica.");
            return;
        }
    }

    // Verificar rango
    bool is_ranged = (w.kind == ItemKind::WEAPON_RANGED || w.kind == ItemKind::WEAPON_MAGIC);
    int  dist      = manhattan(attacker->pos_x, attacker->pos_y, target->pos_x, target->pos_y);
    int  max_dist  = is_ranged ? w.range : 1;
    if (dist > max_dist) {
        world.push_message(client_id, 0, "Objetivo demasiado lejos.");
        return;
    }

    bool crit = is_critical();

    if (!crit && try_dodge(target->agility)) {
        world.push_message(client_id, 0,
            "¡" + std::string(target->username) + " esquivó tu ataque!");
        world.push_message(target_id, 1,
            "Esquivaste el ataque de " + std::string(attacker->username) + "!");
        attacker->attack_cooldown = 10;
        return;
    }

    // Consumir mana del arma mágica
    if (w.kind == ItemKind::WEAPON_MAGIC) {
        attacker->mp -= w.mana_cost;
    }

    uint16_t damage = calc_weapon_damage(*attacker, w);
    if (crit) damage *= 2;

    if (!crit) {
        uint16_t def = calc_defense(*target);
        damage = (damage > def) ? damage - def : 1;
    }

    uint32_t xp_gained = exp_per_damage(damage, attacker->level, target->level);
    attacker->exp += xp_gained;
    check_level_up(*attacker, world);

    std::string crit_str = crit ? " [CRITICO]" : "";
    world.push_message(client_id, 1,
        "Hiciste " + std::to_string(damage) + " de daño" + crit_str + ".");
    world.push_message(target_id, 1,
        "Recibiste " + std::to_string(damage) + " de daño de " +
        std::string(attacker->username) + crit_str + ".");

    world.clan_notify_attack(target_id);

    if (target->hp <= damage) {
        target->hp         = 0;
        target->is_ghost   = true;
        target->meditating = false;
        world.update_occupied({target->pos_x, target->pos_y}, false);
        world.drop_player_loot(*target);

        attacker->exp += exp_on_kill(target->max_hp, attacker->level, target->level);
        check_level_up(*attacker, world);

        world.push_message(target_id, 1, "¡Moriste! Eres un fantasma.");
        world.push_message(client_id, 1, "¡Mataste a " + std::string(target->username) + "!");
    } else {
        target->hp -= damage;
    }

    attacker->attack_cooldown = 10;
}

// AttackNpcCommand

AttackNpcCommand::AttackNpcCommand(uint16_t c, uint16_t n)
    : client_id(c), npc_id(n) {}

void AttackNpcCommand::execute(World& world) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    NpcData*    npc      = world.find_npc(npc_id);

    if (!attacker || !npc) return;
    if (attacker->is_ghost) return;
    if (attacker->attack_cooldown > 0) return;

    attacker->meditating = false;

    const NpcTemplate& tpl = Npcs::tpl(npc->type);

    WeaponInfo w = read_weapon(*attacker);

    if (w.kind == ItemKind::WEAPON_MAGIC) {
        if (attacker->mp < w.mana_cost) {
            world.push_message(client_id, 0,
                "No tenés maná suficiente para disparar el arma mágica.");
            return;
        }
    }

    bool is_ranged = (w.kind == ItemKind::WEAPON_RANGED || w.kind == ItemKind::WEAPON_MAGIC);
    int  dist      = manhattan(attacker->pos_x, attacker->pos_y, npc->pos_x, npc->pos_y);
    int  max_dist  = is_ranged ? w.range : 1;
    if (dist > max_dist) {
        world.push_message(client_id, 0, "El NPC está demasiado lejos.");
        return;
    }

    bool crit = is_critical();

    if (!crit && try_dodge(10)) {
        world.push_message(client_id, 1, "¡El NPC esquivó tu ataque!");
        attacker->attack_cooldown = 10;
        return;
    }

    if (w.kind == ItemKind::WEAPON_MAGIC) {
        attacker->mp -= w.mana_cost;
    }

    uint16_t damage = calc_weapon_damage(*attacker, w);
    if (crit) damage *= 2;

    if (!crit) {
        int def = rand_range(tpl.defense_min, tpl.defense_max);
        damage = (damage > (uint16_t)def) ? damage - (uint16_t)def : 1;
    }

    uint32_t xp = exp_per_damage(damage, attacker->level, 1);
    attacker->exp += xp;
    check_level_up(*attacker, world);

    std::string crit_str = crit ? " [CRITICO]" : "";
    world.push_message(client_id, 1,
        "Hiciste " + std::to_string(damage) + " de daño al " + tpl.name + crit_str + ".");

    if (npc->hp <= damage) {
        uint32_t xp_kill = exp_on_kill(npc->max_hp, attacker->level, 1);
        attacker->exp += xp_kill + tpl.exp_reward;
        check_level_up(*attacker, world);

        uint32_t gold = gold_drop_npc(npc->max_hp);
        attacker->gold += gold;

        npc_drop(*npc, tpl, world);

        world.push_message(client_id, 1,
            "¡Mataste al " + tpl.name + "! +" + std::to_string(gold) + " oro.");

        world.update_occupied({npc->pos_x, npc->pos_y}, false);
        npc->hp = 0;
    } else {
        npc->hp -= damage;
    }

    attacker->attack_cooldown = 10;
}
