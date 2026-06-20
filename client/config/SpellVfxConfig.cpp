#include "SpellVfxConfig.h"
#include <toml++/toml.h>
#include <iostream>

// Valores por defecto
const SpellVfxConfig::SpellEffect SpellVfxConfig::DEFAULT_SPELL_EFFECT = {
    "", 0, 0, 0, {}
};

const SpellVfxConfig::SpellRender SpellVfxConfig::DEFAULT_SPELL_RENDER = {
    0, 0, 0, 0, 1.0f
};

const SpellVfxConfig::SpellVFX SpellVfxConfig::DEFAULT_SPELL_VFX = {
    DEFAULT_SPELL_EFFECT,
    DEFAULT_SPELL_RENDER
};

SpellVfxConfig& SpellVfxConfig::instance() {
    static SpellVfxConfig instance;
    return instance;
}

bool SpellVfxConfig::load(const std::string& config_path) {
    try {
        auto config = toml::parse_file(config_path);

        // Cargar cada spell de spell_effect.X
        for (int i = 1; i <= 9; i++) {
            std::string effect_key = "spell_effect." + std::to_string(i);
            std::string render_key = "spell_render." + std::to_string(i);

            if (auto effect_table = config[effect_key]; effect_table) {
                SpellVFX vfx;

                // Cargar effect
                vfx.effect.path = effect_table["path"].value_or(std::string(""));
                vfx.effect.sheet_cols = effect_table["sheet_cols"].value_or(4);
                vfx.effect.frame_w = effect_table["frame_w"].value_or(32);
                vfx.effect.frame_h = effect_table["frame_h"].value_or(32);

                // Cargar frame_indices (array de TOML)
                if (auto indices_array = effect_table["frame_indices"].as_array()) {
                    for (auto& elem : *indices_array) {
                        if (auto idx = elem.as_integer()) {
                            vfx.effect.frame_indices.push_back(static_cast<int>(idx->get()));
                        }
                    }
                }

                // Cargar render
                if (auto render_table = config[render_key]; render_table) {
                    vfx.render.display_w = render_table["display_w"].value_or(64);
                    vfx.render.display_h = render_table["display_h"].value_or(64);
                    vfx.render.offset_x = render_table["offset_x"].value_or(0);
                    vfx.render.offset_y = render_table["offset_y"].value_or(0);
                    vfx.render.animation_speed = render_table["animation_speed"].value_or(1.0f);
                }

                spells[i] = vfx;
            }
        }

        std::cout << "[SpellVfxConfig] Configuración cargada exitosamente desde: " << config_path 
                  << " (" << spells.size() << " spells)" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SpellVfxConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}

const SpellVfxConfig::SpellVFX& SpellVfxConfig::get_spell_vfx(uint8_t spell_id) const {
    auto it = spells.find(spell_id);
    if (it != spells.end()) {
        return it->second;
    }
    return DEFAULT_SPELL_VFX;
}

const SpellVfxConfig::SpellEffect& SpellVfxConfig::get_effect_info(uint8_t spell_id) const {
    auto it = spells.find(spell_id);
    if (it != spells.end()) {
        return it->second.effect;
    }
    return DEFAULT_SPELL_EFFECT;
}

const SpellVfxConfig::SpellRender& SpellVfxConfig::get_render_info(uint8_t spell_id) const {
    auto it = spells.find(spell_id);
    if (it != spells.end()) {
        return it->second.render;
    }
    return DEFAULT_SPELL_RENDER;
}

bool SpellVfxConfig::has_spell(uint8_t spell_id) const {
    return spells.find(spell_id) != spells.end();
}
