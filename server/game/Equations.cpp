#include "Equations.h"

#include <algorithm>
#include <cmath>
#include <random>

#include "config/GameConfig.h"

namespace Equations {

static std::mt19937& rng() {
    static std::mt19937 r(std::random_device{}());
    return r;
}

int rand_range(int lo, int hi) {
    if (lo >= hi)
        return lo;
    return std::uniform_int_distribution<int>(lo, hi)(rng());
}

double rand_double(double lo, double hi) {
    if (lo >= hi)
        return lo;
    return std::uniform_real_distribution<double>(lo, hi)(rng());
}

int manhattan_distance(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by) {
    return std::abs(static_cast<int>(ax) - static_cast<int>(bx)) +
           std::abs(static_cast<int>(ay) - static_cast<int>(by));
}

bool is_pvp_allowed(uint8_t attacker_lvl, uint8_t target_lvl) {
    const auto& f = GameConfig::get().formulas();
    if (attacker_lvl <= f.pvp_min_level || target_lvl <= f.pvp_min_level)
        return false;
    return std::abs(static_cast<int>(attacker_lvl) - static_cast<int>(target_lvl)) <=
           f.pvp_max_level_delta;
}

uint16_t calc_weapon_damage(uint16_t strength, WeaponBounds w) {
    return static_cast<uint16_t>(strength * rand_range(w.dmin, w.dmax));
}

bool try_dodge(uint16_t agility) {
    const auto& f = GameConfig::get().formulas();
    double chance = std::min(f.dodge_cap, agility * f.dodge_per_agi);
    return rand_double(0.0, 1.0) < chance;
}

bool is_critical() {
    const auto& f = GameConfig::get().formulas();
    return rand_double(0.0, 1.0) < f.crit_chance;
}

uint16_t apply_defense(uint16_t damage, uint16_t defense) {
    return (damage > defense) ? (damage - defense) : 1;
}

uint16_t calc_spell_damage(uint16_t weapon_base, double multiplier, uint32_t flat_bonus,
                           uint16_t intelligence) {
    uint32_t dmg32 = static_cast<uint32_t>(weapon_base * multiplier) + flat_bonus + intelligence;
    return static_cast<uint16_t>(std::min<uint32_t>(dmg32, 65000));
}

uint32_t exp_per_damage(uint16_t damage, uint8_t my_level, uint8_t other_level) {
    const auto& f = GameConfig::get().formulas();
    int factor = std::max(
            static_cast<int>(other_level) - static_cast<int>(my_level) + f.per_damage_base_factor,
            0);
    return static_cast<uint32_t>(damage) * static_cast<uint32_t>(factor);
}

uint32_t exp_on_kill(uint16_t other_max_hp, uint8_t my_level, uint8_t other_level) {
    const auto& f = GameConfig::get().formulas();
    double r = rand_double(0.0, f.kill_exp_rand_max);
    int factor = std::max(
            static_cast<int>(other_level) - static_cast<int>(my_level) + f.per_damage_base_factor,
            0);
    return static_cast<uint32_t>(r * other_max_hp * factor);
}

uint32_t gold_drop_npc(uint16_t npc_max_hp) {
    const auto& f = GameConfig::get().formulas();
    double r = rand_double(f.gold_drop_min_frac, f.gold_drop_max_frac);
    return static_cast<uint32_t>(r * npc_max_hp);
}

uint32_t exp_required_for_level(uint8_t level) {
    const auto& f = GameConfig::get().formulas();
    return static_cast<uint32_t>(f.exp_base * std::pow(static_cast<double>(level), f.exp_exponent));
}

}  // namespace Equations
