#include "ObjectSupConfig.h"
#include <toml++/toml.hpp>

ObjectSupConfig::ObjectSupConfig(const std::string& toml_path) {
    auto tbl = toml::parse_file(toml_path);
    auto* sec = tbl["object_sup"].as_table();
    if (!sec) return;

    for (auto& [key, val] : *sec) {
        uint16_t id = static_cast<uint16_t>(std::stoi(std::string(key)));
        auto* entry_tbl = val.as_table();
        if (!entry_tbl) continue;

        ObjectSupEntry entry;
        entry.size_tiles  = (*entry_tbl)["size_tiles"].value_or<int>(4);
        entry.width_tiles = (*entry_tbl)["width_tiles"].value_or<int>(int{entry.size_tiles});
        entry.offset_x    = (*entry_tbl)["offset_x"].value_or<int>(0);
        entry.offset_y    = (*entry_tbl)["offset_y"].value_or<int>(0);

        auto* frames_arr = (*entry_tbl)["frames"].as_array();
        if (!frames_arr) continue;

        for (auto& frame_val : *frames_arr) {
            auto path = frame_val.value<std::string>();
            if (path) entry.frames.push_back(*path);
        }
        _entries[id] = std::move(entry);
    }
}

const ObjectSupEntry& ObjectSupConfig::get(uint16_t id) const {
    auto it = _entries.find(id);
    if (it == _entries.end()) return _fallback;
    return it->second;
}