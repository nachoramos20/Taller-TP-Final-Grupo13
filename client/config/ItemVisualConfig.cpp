#include "ItemVisualConfig.h"

#include <iostream>
#include <utility>

#include <toml++/toml.h>

namespace {

std::array<int, 4> parse_int4(toml::node_view<toml::node> node) {
    std::array<int, 4> out{};
    auto* arr = node.as_array();
    if (!arr)
        return out;
    int i = 0;
    for (auto& elem: *arr) {
        if (i >= 4)
            break;
        if (auto iv = elem.as_integer())
            out[i] = static_cast<int>(iv->get());
        i++;
    }
    return out;
}

}  // namespace

ItemVisualConfig& ItemVisualConfig::instance() {
    static ItemVisualConfig instance;
    return instance;
}

bool ItemVisualConfig::load(const std::string& toml_path) {
    try {
        auto config = toml::parse_file(toml_path);

        if (auto* items_tbl = config["item"].as_table()) {
            for (auto& [key, val]: *items_tbl) {
                uint8_t id = static_cast<uint8_t>(std::stoi(std::string(key)));
                auto* entry_tbl = val.as_table();
                if (!entry_tbl)
                    continue;

                ItemVisualEntry entry;
                entry.icon_path = (*entry_tbl)["icon_path"].value_or(std::string(""));
                entry.equip_path = (*entry_tbl)["equip_path"].value_or(std::string(""));
                entry.attack_sound_category =
                        (*entry_tbl)["attack_sound_category"].value_or(std::string(""));
                entry.attack_sound_key = (*entry_tbl)["attack_sound_key"].value_or(std::string(""));
                entry.melee_finisher_sound_key =
                        (*entry_tbl)["melee_finisher_sound_key"].value_or(std::string(""));
                _items[id] = entry;

                if (auto helmet_tbl = (*entry_tbl)["helmet"]; helmet_tbl) {
                    HelmetVisual hv;
                    hv.path = entry.equip_path;
                    hv.src_x = helmet_tbl["src_x"].value_or(0);
                    hv.src_y = helmet_tbl["src_y"].value_or(0);
                    hv.src_w = helmet_tbl["src_w"].value_or(0);
                    hv.src_h = helmet_tbl["src_h"].value_or(0);
                    hv.offset_y[1] = parse_int4(helmet_tbl["offset_y_human"]);
                    hv.offset_y[2] = parse_int4(helmet_tbl["offset_y_elf"]);
                    hv.offset_y[3] = parse_int4(helmet_tbl["offset_y_dwarf"]);
                    hv.offset_y[4] = parse_int4(helmet_tbl["offset_y_gnome"]);
                    hv.offset_x[1] = parse_int4(helmet_tbl["offset_x_human"]);
                    hv.offset_x[2] = parse_int4(helmet_tbl["offset_x_elf"]);
                    hv.offset_x[3] = parse_int4(helmet_tbl["offset_x_dwarf"]);
                    hv.offset_x[4] = parse_int4(helmet_tbl["offset_x_gnome"]);
                    _helmets[id] = std::move(hv);
                }
            }
        }

        if (auto* floor_tbl = config["floor_item_variants"].as_table()) {
            for (auto& [key, val]: *floor_tbl) {
                uint16_t id = static_cast<uint16_t>(std::stoi(std::string(key)));
                auto* arr = val.as_array();
                if (!arr)
                    continue;
                std::vector<std::string> variants;
                for (auto& elem: *arr) {
                    if (auto p = elem.value<std::string>())
                        variants.push_back(*p);
                }
                _floor_variants[id] = std::move(variants);
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ItemVisualConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}

const ItemVisualEntry& ItemVisualConfig::get(uint8_t item_id) const {
    auto it = _items.find(item_id);
    return it != _items.end() ? it->second : _fallback;
}

const HelmetVisual* ItemVisualConfig::get_helmet(uint8_t item_id) const {
    auto it = _helmets.find(item_id);
    return it != _helmets.end() ? &it->second : nullptr;
}

const std::vector<std::string>& ItemVisualConfig::get_floor_variants(
        uint16_t floor_sprite_id) const {
    auto it = _floor_variants.find(floor_sprite_id);
    return it != _floor_variants.end() ? it->second : _floor_fallback;
}
