#include "NpcVisualConfig.h"

#include <iostream>
#include <utility>

#include <toml++/toml.h>

NpcVisualConfig& NpcVisualConfig::instance() {
    static NpcVisualConfig instance;
    return instance;
}

bool NpcVisualConfig::load(const std::string& toml_path) {
    try {
        auto config = toml::parse_file(toml_path);

        if (auto* npcs_tbl = config["npc_visual"].as_table()) {
            for (auto& [key, val]: *npcs_tbl) {
                uint8_t id = static_cast<uint8_t>(std::stoi(std::string(key)));
                auto* entry_tbl = val.as_table();
                if (!entry_tbl)
                    continue;

                NpcVisualEntry entry;
                entry.is_service = (*entry_tbl)["is_service"].value_or(false);
                entry.scale = (*entry_tbl)["scale"].value_or(1.5f);
                std::string mode = (*entry_tbl)["select_mode"].value_or(std::string("entity_id"));
                entry.select_mode = (mode == "position_y") ? NpcVariantSelect::ByPositionY :
                                                             NpcVariantSelect::ByEntityId;
                entry.position_y_threshold = (*entry_tbl)["position_y_threshold"].value_or(50);

                if (auto* variants_arr = (*entry_tbl)["variants"].as_array()) {
                    for (auto& v: *variants_arr) {
                        auto* v_tbl = v.as_table();
                        if (!v_tbl)
                            continue;

                        NpcSheetVariant variant;
                        variant.sheet_path = (*v_tbl)["sheet"].value_or(std::string(""));
                        variant.cols = (*v_tbl)["cols"].value_or(1);
                        variant.rows = (*v_tbl)["rows"].value_or(1);
                        variant.frame_w = (*v_tbl)["frame_w"].value_or(32);
                        variant.frame_h = (*v_tbl)["frame_h"].value_or(32);
                        variant.draw_offset_y = (*v_tbl)["draw_offset_y"].value_or(0);
                        variant.name_label_offset_y = (*v_tbl)["name_label_offset_y"].value_or(2);
                        variant.death_sound_key = (*v_tbl)["death_sound"].value_or(std::string(""));
                        entry.variants.push_back(std::move(variant));
                    }
                }

                _npcs[id] = std::move(entry);
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[NpcVisualConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}

const NpcVisualEntry& NpcVisualConfig::get(uint8_t npc_sprite_id) const {
    auto it = _npcs.find(npc_sprite_id);
    return it != _npcs.end() ? it->second : _fallback;
}

const NpcSheetVariant& NpcVisualConfig::select_variant(uint8_t npc_sprite_id, uint16_t entity_id,
                                                       uint16_t pos_y) const {
    const NpcVisualEntry& entry = get(npc_sprite_id);
    if (entry.variants.empty())
        return _fallback_variant;

    size_t idx = 0;
    if (entry.select_mode == NpcVariantSelect::ByPositionY) {
        idx = (pos_y > static_cast<uint16_t>(entry.position_y_threshold)) ? 1 : 0;
        if (idx >= entry.variants.size())
            idx = entry.variants.size() - 1;
    } else {
        idx = entity_id % entry.variants.size();
    }
    return entry.variants[idx];
}
