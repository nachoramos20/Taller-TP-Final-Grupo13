#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include "../../common/protocol/protocol.h"

// Factores y stats iniciales basados en el enunciado de Argentum.
// Los valores son los típicos del TP — afinálos si tu cátedra te dio otros.
struct RaceFactors {
    float hp_factor;
    float mp_factor;
    uint16_t base_str;
    uint16_t base_agi;
    uint16_t base_int;
    uint16_t base_const;
};

struct ClassFactors {
    float hp_factor;
    float mp_factor;
    bool  can_use_weapon_melee;
    bool  can_use_weapon_ranged;
    bool  can_use_armor_heavy;
    bool  can_meditate;
};

namespace Stats {

inline RaceFactors race_of(uint8_t race) {
    switch (static_cast<Race>(race)) {
        case Race::HUMAN:  return {1.0f, 1.0f, 18, 18, 18, 18};
        case Race::ELF:    return {1.0f, 1.4f, 14, 22, 22, 18};
        case Race::DWARF:  return {1.4f, 0.8f, 22, 14, 14, 22};
        case Race::GNOME:  return {0.8f, 1.6f, 14, 18, 24, 16};
    }
    return {1.0f, 1.0f, 18, 18, 18, 18};
}

inline ClassFactors class_of(uint8_t cls) {
    switch (static_cast<Class>(cls)) {
        case Class::MAGE:    return {0.8f, 1.8f, false, true,  false, true};
        case Class::CLERIC:  return {1.0f, 1.4f, true,  true,  false, true};
        case Class::PALADIN: return {1.2f, 1.2f, true,  true,  true,  true};
        case Class::WARRIOR: return {1.5f, 0.6f, true,  true,  true,  false};
    }
    return {1.0f, 1.0f, true, true, true, false};
}

inline uint16_t initial_max_hp(uint8_t race, uint8_t cls) {
    auto r = race_of(race);
    auto c = class_of(cls);
    return static_cast<uint16_t>(100 * r.hp_factor * c.hp_factor);
}

inline uint16_t initial_max_mp(uint8_t race, uint8_t cls) {
    auto r = race_of(race);
    auto c = class_of(cls);
    return static_cast<uint16_t>(100 * r.mp_factor * c.mp_factor);
}

} // namespace Stats

#endif
