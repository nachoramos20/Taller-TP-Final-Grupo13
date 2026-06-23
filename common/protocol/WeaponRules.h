#pragma once

#include <cstdint>
#include <unordered_map>
#include "protocol.h"

// Reglas de armas compartidas entre client y server. Los valores de rango y
// costo de maná espejan items.toml (server) para que el cliente pueda
// predecir localmente si un ataque es válido antes de recibir el snapshot.

// Devuelve true si el item habilita el lanzamiento de hechizos
// (flauta, báculo o vara mágica)
inline bool weapon_enables_spells(uint8_t item_id) {
    return item_id == static_cast<uint8_t>(ItemId::ELVEN_FLUTE)  ||
           item_id == static_cast<uint8_t>(ItemId::GEMMED_STAFF) ||
           item_id == static_cast<uint8_t>(ItemId::ASH_STICK)    ||
           item_id == static_cast<uint8_t>(ItemId::NUDOSO_STAFF);
}

// Devuelve true si el item es un arma mágica que consume mana en ataque básico
inline bool weapon_is_magic(uint8_t item_id) {
    return weapon_enables_spells(item_id);
}

// Devuelve true si el item es un arma a distancia (arco o mágica), para
// decidir si el ataque básico dispara una animación de proyectil.
inline bool weapon_is_ranged(uint8_t item_id) {
    return item_id == static_cast<uint8_t>(ItemId::SIMPLE_BOW)   ||
           item_id == static_cast<uint8_t>(ItemId::COMPOUND_BOW) ||
           weapon_is_magic(item_id);
}

// Patrón aplicado: tabla de lookup única (en vez de 2 switches separados
// sobre el mismo ItemId, uno para rango y otro para costo de maná). Cada
// arma a distancia/mágica es UNA fila con ambos valores juntos, en vez de
// tener que mantener dos switches en sincronía.
struct WeaponClientRules { int range; uint16_t mana_cost; };

inline const std::unordered_map<uint8_t, WeaponClientRules>& weapon_client_rules_table() {
    static const std::unordered_map<uint8_t, WeaponClientRules> table = {
        {static_cast<uint8_t>(ItemId::SIMPLE_BOW),   {6, 0}},
        {static_cast<uint8_t>(ItemId::COMPOUND_BOW), {8, 0}},
        {static_cast<uint8_t>(ItemId::ELVEN_FLUTE),  {6, 100}},
        {static_cast<uint8_t>(ItemId::ASH_STICK),    {5, 5}},
        {static_cast<uint8_t>(ItemId::NUDOSO_STAFF), {6, 15}},
        {static_cast<uint8_t>(ItemId::GEMMED_STAFF), {6, 30}},
    };
    return table;
}

// Rango de ataque del arma en tiles (client-side, espejado de items.toml).
// Usado para validación visual antes de spawnear proyectiles/hechizos.
inline int weapon_client_range(uint8_t item_id) {
    std::unordered_map<uint8_t, WeaponClientRules>::const_iterator it =
        weapon_client_rules_table().find(item_id);
    return it != weapon_client_rules_table().end() ? it->second.range : 1;
}

// Costo de maná del ataque básico con armas mágicas (client-side, espejado
// de items.toml). Usado para no spawnear el proyectil si claramente no hay
// maná suficiente: antes solo se chequeaba "mp > 0", lo que dejaba disparar
// con menos maná del que la propia arma cuesta.
inline uint16_t weapon_client_mana_cost(uint8_t item_id) {
    std::unordered_map<uint8_t, WeaponClientRules>::const_iterator it =
        weapon_client_rules_table().find(item_id);
    return it != weapon_client_rules_table().end() ? it->second.mana_cost : 0;
}
