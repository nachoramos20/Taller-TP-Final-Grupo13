#include "Commands.h"
#include "../Equations.h"
#include "../GameConfig.h"
#include "../Items.h"
#include <algorithm>

// MeditateCommand y ResurrectCommand se mantienen estructuralmente similares pero limpios de lógica local

MeditateCommand::MeditateCommand(uint16_t c) : client_id(c) {}

void MeditateCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    const ClassConfig& cc = GameConfig::get().cls(p->cls);
    if (!cc.can_meditate) {
        world.push_message(client_id, 0, "Tu clase no puede meditar.");
        return;
    }

    p->meditating = !p->meditating;
    world.push_message(client_id, 0, p->meditating ? "Entraste en meditación." : "Saliste de la meditación.");
}

ResurrectCommand::ResurrectCommand(uint16_t c) : client_id(c) {}

void ResurrectCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || !p->is_ghost) return;

    const CombatFormulas& f = GameConfig::get().formulas();

    world.update_occupied({p->pos_x, p->pos_y}, false);
    p->pos_x      = f.respawn_x;
    p->pos_y      = f.respawn_y;
    p->is_ghost   = false;
    p->meditating = false;
    p->hp         = static_cast<uint16_t>(p->max_hp * f.hp_fraction);
    p->mp         = 0;
    world.update_occupied({p->pos_x, p->pos_y}, true);
    world.push_message(client_id, 0, "Resucitaste junto al sanador en la ciudad.");
}

static uint16_t weapon_base_damage(const PlayerData& p) {
    WeaponBounds bounds = {1, 3};
    if (p.equipped_weapon != 0xFF && p.equipped_weapon < PlayerData::INVENTORY_SIZE) {
        uint8_t item_id = p.inventory[p.equipped_weapon];
        if (item_id != 0 && Items::exists(static_cast<ItemId>(item_id))) {
            const ItemDef& def = Items::get(static_cast<ItemId>(item_id));
            bounds = {def.min_value, def.max_value};
        }
    }
    return Equations::calc_weapon_damage(p.strength, bounds);
}

// CastSpellCommand

CastSpellCommand::CastSpellCommand(uint16_t c, uint16_t t, uint8_t s)
    : client_id(c), target_id(t), spell_id(s) {}

void CastSpellCommand::execute(World& world) {
    PlayerData* caster = world.get_player_mutable(client_id);
    if (!caster || caster->is_ghost || caster->attack_cooldown > 0) return;

    if (world.in_safe_zone(caster->pos_x, caster->pos_y)) {
        world.push_message(client_id, 0, "No puedes lanzar hechizos desde una zona segura.");
        return;
    }

    const ClassConfig& cc = GameConfig::get().cls(caster->cls);
    if (!cc.can_meditate) {
        world.push_message(client_id, 0, "Tu clase no puede lanzar hechizos.");
        return;
    }

    uint8_t weapon_item = 0;
    if (caster->equipped_weapon != 0xFF && caster->equipped_weapon < PlayerData::INVENTORY_SIZE)
        weapon_item = caster->inventory[caster->equipped_weapon];

    if (!weapon_enables_spells(weapon_item)) {
        world.push_message(client_id, 0, "Necesitás un báculo, vara o flauta mágica para lanzar hechizos.");
        return;
    }

    if (spell_id == 0) return;

    const SpellConfig& sd = GameConfig::get().spell(spell_id);
    if (sd.spell_class != caster->cls) {
        world.push_message(client_id, 0, "Tu clase no puede lanzar ese hechizo.");
        return;
    }

    // BUG FIX #1: verificar maná ANTES de hacer cualquier cosa
    // (antes esto ya estaba, pero se deja explícito con mensaje claro)
    if (caster->mp < sd.mana_cost) {
        world.push_message(client_id, 0, "Maná insuficiente para " + sd.name + ".");
        return;
    }

    PlayerData* target_p = world.get_player_mutable(target_id);
    NpcData* target_n = target_p ? nullptr : world.find_npc(target_id);
    if (!target_p && !target_n) return;

    uint16_t tx = target_p ? target_p->pos_x : target_n->pos_x;
    uint16_t ty = target_p ? target_p->pos_y : target_n->pos_y;

    if (world.in_safe_zone(tx, ty)) {
        world.push_message(client_id, 0, "No puedes lanzar hechizos contra alguien que está en una zona segura.");
        return;
    }

    if (target_p) {
        if (target_p->is_ghost) return;
        if (world.same_clan(client_id, target_id)) {
            world.push_message(client_id, 0, "No puedes atacar a un compañero de clan.");
            return;
        }
        if (!Equations::is_pvp_allowed(caster->level, target_p->level)) {
            world.push_message(client_id, 0, "No puedes atacar a ese jugador (fair-play).");
            return;
        }
    }

    // BUG FIX #1 (cont.): verificar rango ANTES de consumir maná/disparar animación
    if (Equations::manhattan_distance(caster->pos_x, caster->pos_y, tx, ty) > sd.range) {
        world.push_message(client_id, 0, "Objetivo fuera de alcance para " + sd.name + ".");
        return;
    }

    // Recién acá se confirma el hechizo: salir de meditación y consumir maná
    caster->meditating = false;
    caster->mp -= sd.mana_cost;

    uint16_t base_dmg = weapon_base_damage(*caster);
    uint16_t damage = Equations::calc_spell_damage(base_dmg, sd.dmg_multiplier, sd.flat_bonus, caster->intelligence);

    world.push_message(client_id, 1, "Lanzaste " + sd.name + " e hiciste " + std::to_string(damage) + " de daño.");

    if (target_p) {
        if (target_p->hp <= damage) {
            target_p->hp       = 0;
            target_p->is_ghost = true;
            target_p->meditating = false;
            world.update_occupied({target_p->pos_x, target_p->pos_y}, false);
            world.drop_player_loot(*target_p);
            world.push_message(target_id, 1, "¡Moriste por " + sd.name + "!");

            // EXP por matar jugador con hechizo
            caster->exp += Equations::exp_on_kill(target_p->max_hp, caster->level, target_p->level);
        } else {
            target_p->hp -= damage;
        }
        // EXP por daño a jugador
        caster->exp += Equations::exp_per_damage(damage, caster->level, target_p->level);
    } else if (target_n) {
        if (target_n->hp <= damage) {
            // BUG FIX #2: otorgar EXP al matar NPC con hechizo
            const NpcTemplate& tpl = Npcs::tpl(target_n->type);
            caster->exp += Equations::exp_on_kill(target_n->max_hp, caster->level, 1) + tpl.exp_reward;

            uint32_t gold = Equations::gold_drop_npc(target_n->max_hp);
            caster->gold += gold;

            world.update_occupied({target_n->pos_x, target_n->pos_y}, false);
            target_n->hp = 0;
            world.push_message(client_id, 1, "¡Mataste al objetivo con " + sd.name + "! +" + std::to_string(gold) + " oro.");
        } else {
            target_n->hp -= damage;
            // EXP por daño a NPC
            caster->exp += Equations::exp_per_damage(damage, caster->level, 1);
        }
    }

    caster->attack_cooldown = GameConfig::get().formulas().attack_cooldown_spell;
}
