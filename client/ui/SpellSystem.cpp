#include "SpellSystem.h"

#include <unordered_map>

#include "../../common/protocol/WeaponRules.h"
#include "../../common/protocol/protocol.h"

// Patrón aplicado: tabla de configuración (en vez de switch por Class) —
// agregar un hechizo o una clase nueva es agregar/editar una fila, no un
// case más. Tabla local a la función (static, se construye una sola vez).
std::vector<SpellInfo> SpellSystem::spells_for_class(uint8_t cls) const {
    // BUG FIX anim: rangos espejados del config/spells.toml del servidor.
    // Formato: { spell_id, label, mana_cost, range_tiles }
    static const std::unordered_map<Class, std::vector<SpellInfo>> table = {
            {Class::MAGE,
             {
                     {(uint8_t)SpellId::BURST, "Explosión", 9, 8},
                     {(uint8_t)SpellId::POISON_AREA, "Area de veneno", 18, 6},
                     {(uint8_t)SpellId::SKULL_EXPLOSION, "Explosión calavérica", 32, 6},
             }},
            {Class::CLERIC,
             {
                     {(uint8_t)SpellId::ICE_ORB, "Orbe de hielo", 8, 5},
                     {(uint8_t)SpellId::GRAVITATIONAL_TORNAD, "Tornado gravitatorio", 22, 5},
                     {(uint8_t)SpellId::THUNDERSTORM, "Tormenta eléctrica", 38, 6},
             }},
            {Class::PALADIN,
             {
                     {(uint8_t)SpellId::ORB_OF_EMPTINESS, "Orbe de vacío", 10, 4},
                     {(uint8_t)SpellId::VACUUM_GAP, "Brecha de vacío", 22, 4},
                     {(uint8_t)SpellId::TORNADO_OF_DARKNESS, "Tornado de oscuridad", 40, 2},
             }},
    };

    auto it = table.find(static_cast<Class>(cls));
    return it != table.end() ? it->second : std::vector<SpellInfo>{};
}

uint16_t SpellSystem::selected_spell_mana_cost(uint8_t cls) const {
    if (!_cast_mode || _selected_spell == 0)
        return 0;
    for (const auto& s: spells_for_class(cls))
        if (s.id == _selected_spell)
            return s.mana;
    return 0;
}

int SpellSystem::selected_spell_range(uint8_t cls) const {
    if (!_cast_mode || _selected_spell == 0)
        return 0;
    for (const auto& s: spells_for_class(cls))
        if (s.id == _selected_spell)
            return s.range;
    return 0;
}

void SpellSystem::activate_by_index(uint8_t cls, int index) {
    auto spells = spells_for_class(cls);
    if (index < 0 || index >= (int)spells.size())
        return;
    toggle(spells[index].id);
}

void SpellSystem::toggle(uint8_t spell_id) {
    if (_cast_mode && _selected_spell == spell_id) {
        _cast_mode = false;
        _selected_spell = 0;
    } else {
        _cast_mode = true;
        _selected_spell = spell_id;
    }
}

void SpellSystem::disable_if_weapon_cant_cast(uint8_t eq_weapon_item) {
    if (_cast_mode && !weapon_enables_spells(eq_weapon_item)) {
        _cast_mode = false;
        _selected_spell = 0;
    }
}
