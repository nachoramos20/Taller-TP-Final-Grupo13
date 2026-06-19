#include "Commands.h"
#include "../GameConfig.h"
#include "../Items.h"
#include <algorithm>
#include <random>
#include <cmath>

// MeditateCommand

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
    world.push_message(client_id, 0,
        p->meditating ? "Entraste en meditación." : "Saliste de la meditación.");
}

// ResurrectCommand

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

// Hechizos

namespace {

static std::mt19937& spell_rng() {
    static std::mt19937 r(std::random_device{}());
    return r;
}

static int srand_range(int lo, int hi) {
    if (lo >= hi) return lo;
    return std::uniform_int_distribution<int>(lo, hi)(spell_rng());
}

static int s_manhattan(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by) {
    return std::abs((int)ax - (int)bx) + std::abs((int)ay - (int)by);
}

// Calcula el daño base del arma equipada.
// Para hechizos se usa como base antes de aplicar el multiplicador del spell.
static uint16_t weapon_base_damage(const PlayerData& p) {
    int dmin = 1, dmax = 3;
    if (p.equipped_weapon != 0xFF && p.equipped_weapon < PlayerData::INVENTORY_SIZE) {
        uint8_t item_id = p.inventory[p.equipped_weapon];
        if (item_id != 0 && Items::exists(static_cast<ItemId>(item_id))) {
            const ItemDef& def = Items::get(static_cast<ItemId>(item_id));
            dmin = def.min_value;
            dmax = def.max_value;
        }
    }
    return static_cast<uint16_t>(p.strength * srand_range(dmin, dmax));
}

static bool fair_play_ok(const PlayerData& a, const PlayerData& b) {
    const auto& f = GameConfig::get().formulas();
    if (a.level <= f.pvp_min_level || b.level <= f.pvp_min_level) return false;
    return std::abs((int)a.level - (int)b.level) <= f.pvp_max_level_delta;
}

} // namespace

// CastSpellCommand

CastSpellCommand::CastSpellCommand(uint16_t c, uint16_t t, uint8_t s)
    : client_id(c), target_id(t), spell_id(s) {}

void CastSpellCommand::execute(World& world) {
    PlayerData* caster = world.get_player_mutable(client_id);
    if (!caster || caster->is_ghost) return;
    if (caster->attack_cooldown > 0) return;

    if (world.in_safe_zone(caster->pos_x, caster->pos_y)) {
        world.push_message(client_id, 0, "No puedes lanzar hechizos desde una zona segura.");
        return;
    }

    const ClassConfig& cc = GameConfig::get().cls(caster->cls);
    if (!cc.can_meditate) {   // can_meditate = false solo para Guerrero
        world.push_message(client_id, 0, "Tu clase no puede lanzar hechizos.");
        return;
    }

    // Requiere arma mágica equipada
    uint8_t weapon_item = 0;
    if (caster->equipped_weapon != 0xFF && caster->equipped_weapon < PlayerData::INVENTORY_SIZE)
        weapon_item = caster->inventory[caster->equipped_weapon];

    if (!weapon_enables_spells(weapon_item)) {
        world.push_message(client_id, 0,
            "Necesitás un báculo, vara o flauta mágica para lanzar hechizos.");
        return;
    }

    if (spell_id == 0) {
        world.push_message(client_id, 0, "Hechizo inválido.");
        return;
    }

    // Cargar definición del hechizo desde GameConfig (spells.toml)
    const SpellConfig& sd = GameConfig::get().spell(spell_id);

    if (sd.spell_class != caster->cls) {
        world.push_message(client_id, 0, "Tu clase no puede lanzar ese hechizo.");
        return;
    }

    if (caster->mp < sd.mana_cost) {
        world.push_message(client_id, 0,
            "Maná insuficiente para " + sd.name + ".");
        return;
    }

    // Localizar objetivo
    PlayerData* target_p = world.get_player_mutable(target_id);
    NpcData*    target_n = target_p ? nullptr : world.find_npc(target_id);
    if (!target_p && !target_n) return;

    uint16_t tx = target_p ? target_p->pos_x : target_n->pos_x;
    uint16_t ty = target_p ? target_p->pos_y : target_n->pos_y;

    if (world.in_safe_zone(tx, ty)) {
        world.push_message(client_id, 0,
            "No puedes lanzar hechizos contra alguien que está en una zona segura.");
        return;
    }

    if (target_p) {
        if (target_p->is_ghost) return;

        if (world.same_clan(client_id, target_id)) {
            world.push_message(client_id, 0, "No puedes atacar a un compañero de clan.");
            return;
        }

        if (!fair_play_ok(*caster, *target_p)) {
            world.push_message(client_id, 0, "No puedes atacar a ese jugador (fair-play).");
            return;
        }
    }

    if (s_manhattan(caster->pos_x, caster->pos_y, tx, ty) > sd.range) {
        world.push_message(client_id, 0,
            "Objetivo fuera de alcance para " + sd.name + ".");
        return;
    }

    // Ejecutar hechizo
    caster->meditating = false;
    caster->mp -= sd.mana_cost;

    uint16_t base  = weapon_base_damage(*caster);
    uint32_t dmg32 = static_cast<uint32_t>(base * sd.dmg_multiplier) + sd.flat_bonus;
    dmg32 += caster->intelligence;   // bonus INT
    uint16_t damage = static_cast<uint16_t>(std::min<uint32_t>(dmg32, 65000));

    world.push_message(client_id, 1,
        "Lanzaste " + sd.name + " e hiciste " +
        std::to_string(damage) + " de daño.");

    if (target_p) {
        if (target_p->hp <= damage) {
            target_p->hp       = 0;
            target_p->is_ghost = true;
            target_p->meditating = false;
            world.update_occupied({target_p->pos_x, target_p->pos_y}, false);
            world.drop_player_loot(*target_p);
            world.push_message(target_id, 1,
                "¡Moriste por " + sd.name + "!");
        } else {
            target_p->hp -= damage;
        }
    } else if (target_n) {
        if (target_n->hp <= damage) {
            world.update_occupied({target_n->pos_x, target_n->pos_y}, false);
            target_n->hp = 0;
            world.push_message(client_id, 1,
                "¡Mataste al objetivo con " + sd.name + "!");
        } else {
            target_n->hp -= damage;
        }
    }

    caster->attack_cooldown = GameConfig::get().formulas().attack_cooldown_spell;
}
