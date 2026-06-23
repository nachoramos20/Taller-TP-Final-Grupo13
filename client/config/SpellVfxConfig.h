#ifndef SPELL_VFX_CONFIG_H
#define SPELL_VFX_CONFIG_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Animación y sonido del efecto visual de cada hechizo, cargados desde
// spells_vfx.toml — usado por VFXRenderer y GameAudioService.
class SpellVfxConfig {
public:
    struct SpellEffect {
        std::string path;
        int sheet_cols;
        int frame_w;
        int frame_h;
        std::vector<int> frame_indices;
        std::string sound_path;  // sonido al impactar el hechizo (vacío = sin sonido)
    };

    struct SpellRender {
        int display_w;
        int display_h;
        int offset_x;
        int offset_y;
        float animation_speed;
    };

    struct SpellVFX {
        SpellEffect effect;
        SpellRender render;
    };

    static SpellVfxConfig& instance();

    bool load(const std::string& config_path);

    const SpellVFX& get_spell_vfx(uint8_t spell_id) const;
    const SpellEffect& get_effect_info(uint8_t spell_id) const;
    const SpellRender& get_render_info(uint8_t spell_id) const;
    bool has_spell(uint8_t spell_id) const;

private:
    std::unordered_map<uint8_t, SpellVFX> spells;

    static const SpellVFX DEFAULT_SPELL_VFX;
    static const SpellEffect DEFAULT_SPELL_EFFECT;
    static const SpellRender DEFAULT_SPELL_RENDER;

    SpellVfxConfig() = default;
    ~SpellVfxConfig() = default;

    SpellVfxConfig(const SpellVfxConfig&) = delete;
    SpellVfxConfig& operator=(const SpellVfxConfig&) = delete;
};

#endif  // SPELL_VFX_CONFIG_H
