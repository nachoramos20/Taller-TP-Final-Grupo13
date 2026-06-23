#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Datos de casco: rect de origen en el spritesheet + offset por raza
// (índice 1=humano, 2=elfo, 3=enano, 4=gnomo; 0 sin uso) y dirección
// (orden de columnas: sur, norte, oeste, este — igual que AnimationSystem::DIR_*).
struct HelmetVisual {
    std::string path;
    int src_x = 0, src_y = 0, src_w = 0, src_h = 0;
    std::array<std::array<int, 4>, 5> offset_y{};
    std::array<std::array<int, 4>, 5> offset_x{};
};

struct ItemVisualEntry {
    std::string icon_path;    // ícono mostrado en el inventario
    std::string equip_path;   // overlay dibujado sobre el personaje (vacío = no se equipa visualmente)

    // Sonido al atacar con este ítem (si es un arma). category indica en qué
    // tabla de AudioConfig buscar "key": "melee" | "ranged" | "magic".
    std::string attack_sound_category;
    std::string attack_sound_key;
    std::string melee_finisher_sound_key;  // sonido extra al dar el golpe final (vacío = ninguno)
};

// Config de todo lo visual/sonoro de items que GameLoop necesita para
// renderizar inventario, equipo sobre el personaje, ítems en el piso, y
// resolver qué sonido de ataque corresponde a cada arma.
class ItemVisualConfig {
public:
    static ItemVisualConfig& instance();

    bool load(const std::string& toml_path);

    const ItemVisualEntry& get(uint8_t item_id) const;
    const HelmetVisual* get_helmet(uint8_t item_id) const;  // nullptr si no es casco
    const std::vector<std::string>& get_floor_variants(uint16_t floor_sprite_id) const;

    // Para registrar de una vez todos los íconos de inventario conocidos.
    const std::unordered_map<uint8_t, ItemVisualEntry>& all_items() const { return _items; }

private:
    std::unordered_map<uint8_t, ItemVisualEntry> _items;
    std::unordered_map<uint8_t, HelmetVisual>    _helmets;
    std::unordered_map<uint16_t, std::vector<std::string>> _floor_variants;

    ItemVisualEntry _fallback;
    std::vector<std::string> _floor_fallback{"assets/sprites/items/oro.png"};

    ItemVisualConfig() = default;
    ~ItemVisualConfig() = default;
    ItemVisualConfig(const ItemVisualConfig&) = delete;
    ItemVisualConfig& operator=(const ItemVisualConfig&) = delete;
};
