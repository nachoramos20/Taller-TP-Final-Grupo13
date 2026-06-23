#include "SpellVfxConfig.h"
#include <toml++/toml.h>
#include <iostream>

const SpellVfxConfig::SpellEffect SpellVfxConfig::DEFAULT_SPELL_EFFECT = {
    "", 0, 0, 0, {}, ""
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
        toml::table config = toml::parse_file(config_path);

        // Recorre spell_effect.
        for (int i = 1; i <= 9; i++) {
            std::string idx_str = std::to_string(i);

            if (toml::node_view<toml::node> effect_table = config["spell_effect"][idx_str]; effect_table) {
                SpellVFX vfx;

                vfx.effect.path = effect_table["path"].value_or(std::string(""));
                vfx.effect.sheet_cols = effect_table["sheet_cols"].value_or(4);
                vfx.effect.frame_w = effect_table["frame_w"].value_or(32);
                vfx.effect.frame_h = effect_table["frame_h"].value_or(32);
                vfx.effect.sound_path = effect_table["sound"].value_or(std::string(""));

                if (toml::array* indices_array = effect_table["frame_indices"].as_array()) {
                    for (toml::node& elem : *indices_array) {
                        if (toml::value<int64_t>* idx = elem.as_integer()) {
                            vfx.effect.frame_indices.push_back(static_cast<int>(idx->get()));
                        }
                    }
                }

                if (toml::node_view<toml::node> render_table = config["spell_render"][idx_str]; render_table) {
                    vfx.render.display_w = render_table["display_w"].value_or(64);
                    vfx.render.display_h = render_table["display_h"].value_or(64);
                    vfx.render.offset_x = render_table["offset_x"].value_or(0);
                    vfx.render.offset_y = render_table["offset_y"].value_or(0);
                    vfx.render.animation_speed = render_table["animation_speed"].value_or(1.0f);
                }

                spells[i] = vfx;
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SpellVfxConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}

const SpellVfxConfig::SpellVFX& SpellVfxConfig::get_spell_vfx(uint8_t spell_id) const {
    std::unordered_map<uint8_t, SpellVFX>::const_iterator it = spells.find(spell_id);
    if (it != spells.end()) {
        return it->second;
    }
    return DEFAULT_SPELL_VFX;
}

const SpellVfxConfig::SpellEffect& SpellVfxConfig::get_effect_info(uint8_t spell_id) const {
    std::unordered_map<uint8_t, SpellVFX>::const_iterator it = spells.find(spell_id);
    if (it != spells.end()) {
        return it->second.effect;
    }
    return DEFAULT_SPELL_EFFECT;
}

const SpellVfxConfig::SpellRender& SpellVfxConfig::get_render_info(uint8_t spell_id) const {
    std::unordered_map<uint8_t, SpellVFX>::const_iterator it = spells.find(spell_id);
    if (it != spells.end()) {
        return it->second.render;
    }
    return DEFAULT_SPELL_RENDER;
}

bool SpellVfxConfig::has_spell(uint8_t spell_id) const {
    return spells.find(spell_id) != spells.end();
}
