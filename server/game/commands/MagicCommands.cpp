#include "Commands.h"
#include <algorithm>
#include <random>
#include <cmath>
#include "../Items.h"

MeditateCommand::MeditateCommand(uint16_t c) : client_id(c) {}

void MeditateCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    if (static_cast<Class>(p->cls) == Class::WARRIOR) {
        world.push_message(client_id, 0, "El Guerrero no puede meditar.");
        return;
    }

    p->meditating = !p->meditating;
    if (p->meditating)
        world.push_message(client_id, 0, "Entraste en meditación.");
    else
        world.push_message(client_id, 0, "Saliste de la meditación.");
}

ResurrectCommand::ResurrectCommand(uint16_t c) : client_id(c) {}

void ResurrectCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || !p->is_ghost) return;

    world.update_occupied({p->pos_x, p->pos_y}, false);
    p->pos_x      = 5;
    p->pos_y      = 5;
    p->is_ghost   = false;
    p->meditating = false;
    p->hp         = p->max_hp / 4;
    p->mp         = 0;
    world.update_occupied({p->pos_x, p->pos_y}, true);
    world.push_message(client_id, 0, "Resucitaste junto al sanador.");
}

namespace {

struct SpellDef {
    const char* name;
    Class       cls;
    uint16_t    mana_cost;
    float       dmg_multiplier;   // se aplica sobre el daño del arma
    uint16_t    flat_bonus;       // sumado al daño final
    int         range;            // alcance en tiles (manhattan)
};

static const SpellDef& spell_def(uint8_t id) {
    static const SpellDef defs[] = {
        // NONE (índice 0, placeholder)
        { "Ninguno",            Class::MAGE,    0,    1.0f, 0,   1 },
        // Mago
        { "Misil Mágico",       Class::MAGE,    8,    2.0f, 10,  6 },
        { "Bola de Fuego",      Class::MAGE,    20,   3.0f, 30,  6 },
        { "Rayo",               Class::MAGE,    35,   4.5f, 50,  8 },
        // Clérigo
        { "Castigo Divino",     Class::CLERIC,  10,   2.0f, 8,   5 },
        { "Llama Sagrada",      Class::CLERIC,  22,   2.8f, 25,  5 },
        { "Tormenta de Luz",    Class::CLERIC,  40,   3.5f, 60,  6 },
        // Paladín
        { "Golpe Sagrado",      Class::PALADIN, 6,    1.8f, 6,   2 },
        { "Lanza de Fe",        Class::PALADIN, 15,   2.5f, 20,  4 },
        { "Juicio",             Class::PALADIN, 30,   3.2f, 45,  4 },
    };
    if (id == 0 || id > 9) return defs[0];
    return defs[id];
}

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

} // namespace

CastSpellCommand::CastSpellCommand(uint16_t c, uint16_t t, uint8_t s)
    : client_id(c), target_id(t), spell_id(s) {}

void CastSpellCommand::execute(World& world) {
    PlayerData* caster = world.get_player_mutable(client_id);
    if (!caster || caster->is_ghost) return;
    if (caster->attack_cooldown > 0) return;

    if (static_cast<Class>(caster->cls) == Class::WARRIOR) {
        world.push_message(client_id, 0, "El Guerrero no puede lanzar hechizos.");
        return;
    }

    // Validar arma mágica equipada
    uint8_t weapon_item = 0;
    if (caster->equipped_weapon != 0xFF && caster->equipped_weapon < PlayerData::INVENTORY_SIZE)
        weapon_item = caster->inventory[caster->equipped_weapon];
    if (!weapon_enables_spells(weapon_item)) {
        world.push_message(client_id, 0, "Necesitás un báculo o vara para lanzar hechizos.");
        return;
    }

    const SpellDef& sd = spell_def(spell_id);
    if (spell_id == 0) {
        world.push_message(client_id, 0, "Hechizo inválido.");
        return;
    }
    if (static_cast<uint8_t>(sd.cls) != caster->cls) {
        world.push_message(client_id, 0, "Tu clase no puede lanzar ese hechizo.");
        return;
    }
    if (caster->mp < sd.mana_cost) {
        world.push_message(client_id, 0, "Maná insuficiente para " + std::string(sd.name) + ".");
        return;
    }

    // Localizar objetivo (puede ser otro player o un NPC)
    PlayerData* target_p = world.get_player_mutable(target_id);
    NpcData*    target_n = target_p ? nullptr : world.find_npc(target_id);
    if (!target_p && !target_n) return;

    uint16_t tx = target_p ? target_p->pos_x : target_n->pos_x;
    uint16_t ty = target_p ? target_p->pos_y : target_n->pos_y;
    if (s_manhattan(caster->pos_x, caster->pos_y, tx, ty) > sd.range) {
        world.push_message(client_id, 0, "Objetivo fuera de alcance para " + std::string(sd.name) + ".");
        return;
    }

    caster->meditating = false;
    caster->mp -= sd.mana_cost;

    uint16_t base = weapon_base_damage(*caster);
    uint32_t dmg32 = static_cast<uint32_t>(base * sd.dmg_multiplier) + sd.flat_bonus;
    // bonus por inteligencia
    dmg32 += caster->intelligence;
    uint16_t damage = static_cast<uint16_t>(std::min<uint32_t>(dmg32, 65000));

    world.push_message(client_id, 1,
        std::string("Lanzaste ") + sd.name + " e hiciste " +
        std::to_string(damage) + " de daño.");

    if (target_p) {
        if (target_p->hp <= damage) {
            target_p->hp = 0;
            target_p->is_ghost = true;
            target_p->meditating = false;
            world.update_occupied({target_p->pos_x, target_p->pos_y}, false);
            world.drop_player_loot(*target_p);
            world.push_message(target_id, 1, "Moriste por " + std::string(sd.name) + "!");
        } else {
            target_p->hp -= damage;
        }
    } else if (target_n) {
        if (target_n->hp <= damage) {
            world.update_occupied({target_n->pos_x, target_n->pos_y}, false);
            target_n->hp = 0;
            world.push_message(client_id, 1, "Mataste al objetivo con " + std::string(sd.name) + "!");
        } else {
            target_n->hp -= damage;
        }
    }

    caster->attack_cooldown = 12;
}
