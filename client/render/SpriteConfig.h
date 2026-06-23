#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

// Sprite/recorte de un tile de piso, tal como vienen de tiles.toml.
struct TileEntry {
    std::string path;
    int src_x = 0;
    int src_y = 0;
    int src_w = 0;  // 0 = usar imagen completa
    int src_h = 0;
    int tile_size = 1;  // cuántos tiles ocupa (1 = normal, 6 = acantilado)
    int offset_x = 0;
    int offset_y = 0;

    bool has_src_rect() const { return src_w > 0; }
    bool is_large() const { return tile_size > 1; }
};

// Tiles de piso definidos en tiles.toml, indexados por floor_id.
class TileConfig {
public:
    TileConfig(const std::string& toml_path, const std::string& section);
    const TileEntry& get(uint16_t id) const;

private:
    std::unordered_map<uint16_t, TileEntry> _entries;
    TileEntry _fallback;
};

// Rutas de body/head para un sprite_id (raza), tal como vienen de sprites.toml.
struct SpriteEntry {
    std::string body_path;
    std::string head_path;
};

// Sprites de personaje definidos en sprites.toml, indexados por sprite_id.
class SpriteConfig {
public:
    explicit SpriteConfig(const std::string& toml_path);
    const SpriteEntry& get(uint8_t sprite_id) const;

private:
    std::unordered_map<uint8_t, SpriteEntry> _entries;
    SpriteEntry _fallback;
};
