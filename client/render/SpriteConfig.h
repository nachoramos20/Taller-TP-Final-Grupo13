#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

// Entrada para sprites de personaje (body + head)
struct SpriteEntry {
    std::string body_path;
    std::string head_path;
};

// Mapea uint16_t id -> path leyendo [section.id] de un TOML
class TileConfig {
public:
    TileConfig(const std::string& toml_path, const std::string& section);

    // Devuelve la ruta para el id dado. Si no existe devuelve "".
    const std::string& get(uint16_t id) const;

private:
    std::unordered_map<uint16_t, std::string> _entries;
    std::string _fallback;
};

// Mapea uint8_t sprite_id -> SpriteEntry (body + head)
class SpriteConfig {
public:
    explicit SpriteConfig(const std::string& toml_path);

    const SpriteEntry& get(uint8_t sprite_id) const;

private:
    std::unordered_map<uint8_t, SpriteEntry> _entries;
    SpriteEntry _fallback;
};