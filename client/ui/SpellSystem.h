#pragma once

#include <cstdint>
#include <vector>

class GameAudioService;

// BUG FIX anim: agregado campo `range` para validación client-side.
struct SpellInfo { uint8_t id; const char* label; uint16_t mana; int range; };

// Qué hechizos tiene cada clase, cuál está seleccionado y si el "modo
// hechizo" está activo. Extraída de StatsPanel, que mezclaba esto con
// todo el dibujado del panel — esta lógica no dibuja nada, solo la
// consultan StatsPanelRenderer (para pintar el botón resaltado) y
// PlayerActionController (para decidir si el click lanza un hechizo).
class SpellSystem {
public:
    std::vector<SpellInfo> spells_for_class(uint8_t cls) const;

    bool    cast_mode_active() const { return _cast_mode; }
    uint8_t selected_spell()  const { return _selected_spell; }

    uint16_t selected_spell_mana_cost(uint8_t cls) const;
    int      selected_spell_range(uint8_t cls) const;

    // Atajo de teclado 1-2-3: activa por índice dentro de spells_for_class(cls).
    void activate_by_index(uint8_t cls, int index);
    // Click sobre un botón de hechizo ya dibujado (ver StatsPanelRenderer).
    void toggle(uint8_t spell_id);

    // Si el arma equipada deja de habilitar hechizos, apaga el modo
    // (llamado desde StatsPanel::update() en cada snapshot).
    void disable_if_weapon_cant_cast(uint8_t eq_weapon_item);

private:
    bool     _cast_mode      = false;
    uint8_t  _selected_spell = 0;
};
