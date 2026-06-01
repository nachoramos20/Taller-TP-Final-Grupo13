#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

struct SpriteEntry {
    std::string body_path;
    std::string head_path;
};

class SpriteConfig {
public:
    explicit SpriteConfig(const std::string& toml_path);

    const SpriteEntry& get(uint8_t sprite_id) const;

private:
    std::unordered_map<uint8_t, SpriteEntry> _entries;
    SpriteEntry _fallback;
};