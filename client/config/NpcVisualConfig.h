#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Cómo elegir, de la lista de variantes de un NPC, cuál corresponde a una
// entidad puntual: por entity_id (enemigos, da variedad visual estable por
// entidad) o por posición Y (NPCs de servicio: ciudad vs. pueblo).
enum class NpcVariantSelect { ByEntityId, ByPositionY };

struct NpcSheetVariant {
    std::string sheet_path;
    int cols = 1, rows = 1, frame_w = 32, frame_h = 32;
    int draw_offset_y = 0;
    int name_label_offset_y = 2;
    std::string death_sound_key;  // clave en AudioConfig::creature_sounds (vacío = sin sonido de muerte)
};

struct NpcVisualEntry {
    bool is_service = false;
    float scale = 1.5f;
    NpcVariantSelect select_mode = NpcVariantSelect::ByEntityId;
    int position_y_threshold = 50;
    std::vector<NpcSheetVariant> variants;
};

class NpcVisualConfig {
public:
    static NpcVisualConfig& instance();

    bool load(const std::string& toml_path);

    const NpcVisualEntry& get(uint8_t npc_sprite_id) const;

    // Centraliza la selección de variante: la usan tanto el renderer (sprite)
    // como el servicio de audio (sonido de muerte), así no hay dos lugares
    // calculando el mismo "entity_id % N" o el mismo umbral ciudad/pueblo.
    const NpcSheetVariant& select_variant(uint8_t npc_sprite_id, uint16_t entity_id, uint16_t pos_y) const;

private:
    std::unordered_map<uint8_t, NpcVisualEntry> _npcs;
    NpcVisualEntry   _fallback;
    NpcSheetVariant  _fallback_variant;

    NpcVisualConfig() = default;
    ~NpcVisualConfig() = default;
    NpcVisualConfig(const NpcVisualConfig&) = delete;
    NpcVisualConfig& operator=(const NpcVisualConfig&) = delete;
};
