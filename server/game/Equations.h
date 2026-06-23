#pragma once
#include <cstdint>

// Estructuras livianas para no acoplar el módulo a los comandos enteros si no es necesario
struct WeaponBounds { int dmin; int dmax; };
struct DefenseBounds { int dmin; int dmax; };

// Fórmulas de balance (daño, esquive, experiencia, oro) sin estado propio;
// los parámetros configurables de cada fórmula vienen de GameConfig.
namespace Equations {
    int rand_range(int lo, int hi);
    double rand_double(double lo, double hi);
    int manhattan_distance(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by);
    bool is_pvp_allowed(uint8_t attacker_lvl, uint8_t target_lvl);

    uint16_t calc_weapon_damage(uint16_t strength, WeaponBounds w);
    bool try_dodge(uint16_t agility);
    bool is_critical();
    uint16_t apply_defense(uint16_t damage, uint16_t defense);

    uint16_t calc_spell_damage(uint16_t weapon_base, double multiplier, uint32_t flat_bonus, uint16_t intelligence);

    uint32_t exp_per_damage(uint16_t damage, uint8_t my_level, uint8_t other_level);
    uint32_t exp_on_kill(uint16_t other_max_hp, uint8_t my_level, uint8_t other_level);
    uint32_t gold_drop_npc(uint16_t npc_max_hp);
    uint32_t exp_required_for_level(uint8_t level);
}
