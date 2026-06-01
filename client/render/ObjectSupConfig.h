#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct ObjectSupEntry {
    std::vector<std::string> frames; 
};

class ObjectSupConfig {
public:
    explicit ObjectSupConfig(const std::string& toml_path);

    // Devuelve la entry para el id dado.
    // Si no existe devuelve una entry con frames vacío.
    const ObjectSupEntry& get(uint16_t id) const;

private:
    std::unordered_map<uint16_t, ObjectSupEntry> _entries;
    ObjectSupEntry _fallback;  // frames vacío
};