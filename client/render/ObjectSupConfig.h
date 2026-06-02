#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct ObjectSupEntry {
    std::vector<std::string> frames;
    int size_tiles = 4; 
};

class ObjectSupConfig {
public:
    explicit ObjectSupConfig(const std::string& toml_path);

    const ObjectSupEntry& get(uint16_t id) const;

private:
    std::unordered_map<uint16_t, ObjectSupEntry> _entries;
    ObjectSupEntry _fallback;
};