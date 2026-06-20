#ifndef SPELL_VFX_CONFIG_H
#define SPELL_VFX_CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

class SpellVfxConfig {
public:
    // Estructura para información del efecto de un spell
    struct SpellEffect {
        std::string path;
        int sheet_cols;
        int frame_w;
        int frame_h;
        std::vector<int> frame_indices;
    };

    // Estructura para información de render de un spell
    struct SpellRender {
        int display_w;
        int display_h;
        int offset_x;
        int offset_y;
        float animation_speed;
    };

    // Estructura combinada
    struct SpellVFX {
        SpellEffect effect;
        SpellRender render;
    };

    // Singleton
    static SpellVfxConfig& instance();

    // Cargar configuración desde TOML
    bool load(const std::string& config_path);

    // Getters
    const SpellVFX& get_spell_vfx(uint8_t spell_id) const;
    const SpellEffect& get_effect_info(uint8_t spell_id) const;
    const SpellRender& get_render_info(uint8_t spell_id) const;
    bool has_spell(uint8_t spell_id) const;

private:
    std::unordered_map<uint8_t, SpellVFX> spells;

    // Valores por defecto
    static const SpellVFX DEFAULT_SPELL_VFX;
    static const SpellEffect DEFAULT_SPELL_EFFECT;
    static const SpellRender DEFAULT_SPELL_RENDER;

    SpellVfxConfig() = default;
    ~SpellVfxConfig() = default;

    // Prevenir copia
    SpellVfxConfig(const SpellVfxConfig&) = delete;
    SpellVfxConfig& operator=(const SpellVfxConfig&) = delete;
};

#endif // SPELL_VFX_CONFIG_H
