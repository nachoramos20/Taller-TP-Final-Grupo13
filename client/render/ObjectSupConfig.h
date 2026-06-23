#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Animación (frames) y tamaño/offset de un objeto superior (lo que se
// dibuja por encima del jugador, p. ej. copas de árbol).
struct ObjectSupEntry {
    std::vector<std::string> frames;
    int size_tiles = 1;
    int width_tiles = 1;
    int offset_x = 0;
    int offset_y = 0;
};

// Objetos superiores definidos en objects_sup.toml, indexados por id.
class ObjectSupConfig {
public:
    explicit ObjectSupConfig(const std::string& toml_path);

    const ObjectSupEntry& get(uint16_t id) const;

private:
    std::unordered_map<uint16_t, ObjectSupEntry> _entries;
    ObjectSupEntry _fallback;
};