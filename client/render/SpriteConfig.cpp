#include "SpriteConfig.h"
#include <toml++/toml.hpp>

// TileConfig ---------------------------------------------------------------------

TileConfig::TileConfig(const std::string& toml_path, const std::string& section) {
    auto tbl = toml::parse_file(toml_path);
    auto* sec = tbl[section].as_table();
    if (!sec) return;

    for (auto& [key, val] : *sec) {
        uint16_t id = static_cast<uint16_t>(std::stoi(std::string(key)));
        auto* entry_tbl = val.as_table();
        if (!entry_tbl) continue;

        TileEntry entry;
        entry.path      = (*entry_tbl)["path"].value_or<std::string>("");
        entry.src_x     = (*entry_tbl)["src_x"].value_or<int>(0);
        entry.src_y     = (*entry_tbl)["src_y"].value_or<int>(0);
        entry.src_w     = (*entry_tbl)["src_w"].value_or<int>(0);
        entry.src_h     = (*entry_tbl)["src_h"].value_or<int>(0);
        entry.tile_size = (*entry_tbl)["tile_size"].value_or<int>(1);
        entry.offset_x  = (*entry_tbl)["offset_x"].value_or<int>(0);
        entry.offset_y  = (*entry_tbl)["offset_y"].value_or<int>(0);
        _entries[id] = entry;
    }
}

const TileEntry& TileConfig::get(uint16_t id) const {
    auto it = _entries.find(id);
    if (it == _entries.end()) return _fallback;
    return it->second;
}

// SpriteConfig ----------------------------------------------------------------------

SpriteConfig::SpriteConfig(const std::string& toml_path) {
    _fallback.body_path = "assets/sprites/skins/humano.png";
    _fallback.head_path = "assets/sprites/heads/humano.png";

    auto tbl = toml::parse_file(toml_path);
    auto* sprites = tbl["sprite"].as_table();
    if (!sprites) return;

    for (auto& [key, val] : *sprites) {
        uint8_t id = static_cast<uint8_t>(std::stoi(std::string(key)));
        auto* entry_tbl = val.as_table();
        if (!entry_tbl) continue;

        SpriteEntry entry;
        entry.body_path = (*entry_tbl)["body"].value_or<std::string>(
            std::string(_fallback.body_path));
        entry.head_path = (*entry_tbl)["head"].value_or<std::string>(
            std::string(_fallback.head_path));
        _entries[id] = entry;
    }
}

const SpriteEntry& SpriteConfig::get(uint8_t sprite_id) const {
    auto it = _entries.find(sprite_id);
    if (it == _entries.end()) return _fallback;
    return it->second;
}