#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include "../../common/protocol/protocol.h"
#include "config/GameConfig.h"

// Mantiene compatibilidad con los structs usados en código viejo
using RaceFactors  = RaceConfig;
using ClassFactors = ClassConfig;

namespace Stats {

inline uint16_t initial_max_hp(uint8_t race, uint8_t cls) {
    return GameConfig::get().initial_max_hp(race, cls);
}

inline uint16_t initial_max_mp(uint8_t race, uint8_t cls) {
    return GameConfig::get().initial_max_mp(race, cls);
}

// Acceso directo a los factores de raza/clase (para código que los lea)
inline const RaceConfig& race_of(uint8_t race) {
    return GameConfig::get().race(race);
}

inline const ClassConfig& class_of(uint8_t cls) {
    return GameConfig::get().cls(cls);
}

} // namespace Stats

#endif
