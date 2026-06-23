// El archivo supera las 200 líneas, pero ninguna de las dos clases que
// contiene (AttackCommand::execute ~130, AttackNpcCommand::execute ~85)
// lo hace individualmente — comparten archivo por ser ambas "resolver un
// ataque", igual que MeditateCommand/ResurrectCommand/CastSpellCommand
// en MagicCommands.cpp. Las fórmulas de daño/crítico/dodge/exp ya están
// extraídas a Equations (server/game/Equations.h/.cpp), así que no hay
// un "CombatCalculator" más para sacar de acá.
#include <algorithm>

#include "../Equations.h"
#include "../Items.h"
#include "../config/GameConfig.h"

#include "Commands.h"

struct WeaponInfo {
    ItemKind kind = ItemKind::NONE;
    uint8_t item_id = 0;
    uint16_t mana_cost = 0;
    int range = 1;
    WeaponBounds bounds = {1, 3};
    bool valid = false;
};

static WeaponInfo read_weapon(const PlayerData& p) {
    WeaponInfo w;
    uint8_t slot = p.equipped_weapon;
    if (slot == 0xFF || slot >= PlayerData::INVENTORY_SIZE)
        return w;

    uint8_t item_id = p.inventory[slot];
    if (item_id == 0 || !Items::exists(static_cast<ItemId>(item_id)))
        return w;

    const ItemDef& def = Items::get(static_cast<ItemId>(item_id));
    w.kind = def.kind;
    w.item_id = item_id;
    w.mana_cost = def.mana_cost;
    w.range = (def.range_tiles > 0) ? def.range_tiles : 1;
    w.bounds = {def.min_value, def.max_value};
    w.valid = true;
    return w;
}

static uint16_t calc_defense(const PlayerData& defender) {
    int def = 0;
    auto add_piece = [&](uint8_t slot) {
        if (slot == 0xFF || slot >= PlayerData::INVENTORY_SIZE)
            return;
        uint8_t item_id = defender.inventory[slot];
        if (item_id == 0 || !Items::exists(static_cast<ItemId>(item_id)))
            return;
        const auto& d = Items::get(static_cast<ItemId>(item_id));
        def += Equations::rand_range(d.min_value, d.max_value);
    };

    add_piece(defender.equipped_armor);
    add_piece(defender.equipped_helmet);
    add_piece(defender.equipped_shield);
    return static_cast<uint16_t>(def);
}

static void npc_drop(const NpcData& npc, const NpcTemplate& tpl, World& world) {
    const auto& f = GameConfig::get().formulas();
    double r = Equations::rand_double(0.0, 1.0);
    if (r < f.drop_chance_nothing)
        return;
    if (r < f.drop_chance_gold) {
        uint32_t gold = Equations::gold_drop_npc(tpl.max_hp);
        world.add_floor_item(static_cast<uint8_t>(ItemId::GOLD_PILE), npc.pos_x, npc.pos_y, gold);
        return;
    }
    if (r < f.drop_chance_potion) {
        uint8_t pot = (Equations::rand_range(0, 1) == 0) ?
                              static_cast<uint8_t>(ItemId::HEALTH_POTION) :
                              static_cast<uint8_t>(ItemId::MANA_POTION);
        world.add_floor_item(pot, npc.pos_x, npc.pos_y, 0);
        return;
    }
    if (r < f.drop_chance_item) {
        if (!tpl.drop_table.empty()) {
            int idx = Equations::rand_range(0, (int)tpl.drop_table.size() - 1);
            world.add_floor_item(tpl.drop_table[idx], npc.pos_x, npc.pos_y, 0);
        }
    }
}

AttackCommand::AttackCommand(uint16_t c, uint16_t t): client_id(c), target_id(t) {}

void AttackCommand::execute(World& world) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    PlayerData* target = world.get_player_mutable(target_id);

    if (!attacker)
        return;
    if (!target) {
        if (world.find_npc(target_id)) {
            AttackNpcCommand npc_cmd(client_id, target_id);
            npc_cmd.execute(world);
        }
        return;
    }

    if (attacker->is_ghost || target->is_ghost)
        return;
    if (attacker->is_in_combat())
        return;

    if (world.in_safe_zone(attacker->pos_x, attacker->pos_y) ||
        world.in_safe_zone(target->pos_x, target->pos_y)) {
        world.push_message(client_id, 0, "No se puede atacar en/desde zona segura.");
        return;
    }

    attacker->meditating = false;
    WeaponInfo w = read_weapon(*attacker);
    int dist = Equations::manhattan_distance(attacker->pos_x, attacker->pos_y, target->pos_x,
                                             target->pos_y);

    // La flauta élfica cura a otro jugador en vez de hacer daño,
    // así que se chequea primero.
    if (w.item_id == static_cast<uint8_t>(ItemId::ELVEN_FLUTE)) {
        if (!attacker->cheat_infinite_mp && attacker->mp < w.mana_cost) {
            world.push_message(client_id, 0, "No tenés maná suficiente para tocar la flauta.");
            return;
        }
        if (dist > w.range) {
            world.push_message(client_id, 0, "Objetivo demasiado lejos.");
            return;
        }
        if (!attacker->cheat_infinite_mp)
            attacker->mp -= w.mana_cost;
        uint16_t heal = static_cast<uint16_t>(Equations::rand_range(w.bounds.dmin, w.bounds.dmax));
        target->hp = std::min(target->max_hp, static_cast<uint16_t>(target->hp + heal));
        world.push_message(client_id, 1,
                           "Curaste a " + std::string(target->username) + " por " +
                                   std::to_string(heal) + " de vida.");
        world.push_message(
                target_id, 1,
                std::string(attacker->username) + " te curó " + std::to_string(heal) + " de vida.");
        attacker->attack_cooldown = GameConfig::get().formulas().attack_cooldown_melee;
        world.push_spell_event(client_id, 0, target->pos_x, target->pos_y,
                               /*is_magic_projectile*/ true);
        return;
    }

    if (!Equations::is_pvp_allowed(attacker->level, target->level)) {
        world.push_message(client_id, 0, "No puedes atacar a ese jugador (fair-play).");
        return;
    }
    if (world.same_clan(client_id, target_id)) {
        world.push_message(client_id, 0, "No puedes atacar a un compañero de clan.");
        return;
    }

    if (w.kind == ItemKind::WEAPON_MAGIC && !attacker->cheat_infinite_mp &&
        attacker->mp < w.mana_cost) {
        world.push_message(client_id, 0, "No tenés maná suficiente para disparar el arma mágica.");
        return;
    }

    bool is_ranged = (w.kind == ItemKind::WEAPON_RANGED || w.kind == ItemKind::WEAPON_MAGIC);
    if (dist > (is_ranged ? w.range : 1)) {
        world.push_message(client_id, 0, "Objetivo demasiado lejos.");
        return;
    }

    // El ataque está confirmado a partir de acá (pasó fair-play, clan, maná
    // y rango): si es a distancia, avisarle a los demás clientes para que
    // vean el proyectil del atacante. El propio cliente ya lo spawneó
    // localmente al clickear; esto es solo para los demás. Se manda antes
    // de resolver esquive/crítico porque el proyectil viaja igual aunque
    // termine esquivado.
    if (is_ranged) {
        world.push_spell_event(client_id, 0, target->pos_x, target->pos_y,
                               /*is_magic_projectile*/ w.kind == ItemKind::WEAPON_MAGIC);
    }

    bool crit = Equations::is_critical();

    if (!crit && Equations::try_dodge(target->agility)) {
        world.push_message(client_id, 0,
                           "¡" + std::string(target->username) + " esquivó tu ataque!");
        world.push_message(target_id, 1,
                           "Esquivaste el ataque de " + std::string(attacker->username) + "!");
        attacker->attack_cooldown = GameConfig::get().formulas().attack_cooldown_melee;
        return;
    }

    if (w.kind == ItemKind::WEAPON_MAGIC && !attacker->cheat_infinite_mp)
        attacker->mp -= w.mana_cost;

    uint16_t damage = Equations::calc_weapon_damage(attacker->strength, w.bounds);
    if (crit) {
        damage = static_cast<uint16_t>(damage * GameConfig::get().formulas().crit_multiplier);
    } else {
        damage = Equations::apply_defense(damage, calc_defense(*target));
    }

    attacker->exp += Equations::exp_per_damage(damage, attacker->level, target->level);
    world.check_level_up(*attacker);

    std::string crit_str = crit ? " [CRITICO]" : "";
    world.push_message(client_id, 1,
                       "Hiciste " + std::to_string(damage) + " de daño" + crit_str + ".");
    world.push_message(target_id, 1,
                       "Recibiste " + std::to_string(damage) + " de daño de " +
                               std::string(attacker->username) + crit_str + ".");

    world.clan_notify_attack(target_id);

    if (target->cheat_infinite_hp) {
        // Vida infinita: no recibe daño ni muere.
    } else if (target->hp <= damage) {
        target->hp = 0;
        target->is_ghost = true;
        target->meditating = false;
        world.update_occupied({target->pos_x, target->pos_y}, false);
        world.drop_player_loot(*target);

        attacker->exp += Equations::exp_on_kill(target->max_hp, attacker->level, target->level);
        world.check_level_up(*attacker);

        world.push_message(target_id, 1, "¡Moriste! Eres un fantasma.");
        world.push_message(client_id, 1, "¡Mataste a " + std::string(target->username) + "!");
    } else {
        target->hp -= damage;
    }

    attacker->attack_cooldown = GameConfig::get().formulas().attack_cooldown_melee;
}

AttackNpcCommand::AttackNpcCommand(uint16_t c, uint16_t n): client_id(c), npc_id(n) {}

void AttackNpcCommand::execute(World& world) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    NpcData* npc = world.find_npc(npc_id);

    if (!attacker || !npc)
        return;
    if (attacker->is_ghost || attacker->is_in_combat())
        return;

    if (world.in_safe_zone(attacker->pos_x, attacker->pos_y)) {
        world.push_message(client_id, 0, "No puedes atacar desde una zona segura.");
        return;
    }

    attacker->meditating = false;
    const NpcTemplate& tpl = Npcs::tpl(npc->type);
    WeaponInfo w = read_weapon(*attacker);

    if (w.kind == ItemKind::WEAPON_MAGIC && !attacker->cheat_infinite_mp &&
        attacker->mp < w.mana_cost) {
        world.push_message(client_id, 0, "No tenés maná suficiente para disparar el arma mágica.");
        return;
    }

    bool is_ranged = (w.kind == ItemKind::WEAPON_RANGED || w.kind == ItemKind::WEAPON_MAGIC);
    int dist =
            Equations::manhattan_distance(attacker->pos_x, attacker->pos_y, npc->pos_x, npc->pos_y);
    if (dist > (is_ranged ? w.range : 1)) {
        world.push_message(client_id, 0, "El NPC está demasiado lejos.");
        return;
    }

    // Igual que en AttackCommand: avisar a los demás clientes del proyectil
    // (el atacante ya lo vio localmente al clickear).
    if (is_ranged) {
        world.push_spell_event(client_id, 0, npc->pos_x, npc->pos_y,
                               /*is_magic_projectile*/ w.kind == ItemKind::WEAPON_MAGIC);
    }

    bool crit = Equations::is_critical();
    // Los NPCs esquivan con una probabilidad fija y baja.
    if (!crit && Equations::try_dodge(10)) {
        world.push_message(client_id, 1, "¡El NPC esquivó tu ataque!");
        attacker->attack_cooldown = GameConfig::get().formulas().attack_cooldown_melee;
        return;
    }

    if (w.kind == ItemKind::WEAPON_MAGIC && !attacker->cheat_infinite_mp)
        attacker->mp -= w.mana_cost;

    uint16_t damage = Equations::calc_weapon_damage(attacker->strength, w.bounds);
    if (crit) {
        damage = static_cast<uint16_t>(damage * GameConfig::get().formulas().crit_multiplier);
    } else {
        uint16_t npc_def =
                static_cast<uint16_t>(Equations::rand_range(tpl.defense_min, tpl.defense_max));
        damage = Equations::apply_defense(damage, npc_def);
    }

    attacker->exp += Equations::exp_per_damage(damage, attacker->level, 1);
    world.check_level_up(*attacker);

    std::string crit_str = crit ? " [CRITICO]" : "";
    world.push_message(
            client_id, 1,
            "Hiciste " + std::to_string(damage) + " de daño al " + tpl.name + crit_str + ".");

    if (npc->hp <= damage) {
        attacker->exp += Equations::exp_on_kill(npc->max_hp, attacker->level, 1) + tpl.exp_reward;
        world.check_level_up(*attacker);

        uint32_t gold = Equations::gold_drop_npc(npc->max_hp);
        attacker->gold += gold;

        npc_drop(*npc, tpl, world);
        world.push_message(client_id, 1,
                           "¡Mataste al " + tpl.name + "! +" + std::to_string(gold) + " oro.");

        world.update_occupied({npc->pos_x, npc->pos_y}, false);
        npc->hp = 0;
    } else {
        npc->hp -= damage;
    }

    attacker->attack_cooldown = GameConfig::get().formulas().attack_cooldown_melee;
}
