#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../../../common/protocol/protocol.h"

// Plantilla de NPC (datos de balance cargados de npcs.toml vía GameConfig,
// con cache en Npcs::tpl). Antes vivía junto a las funciones de lookup en
// Npc.h; se usa desde varios comandos (CombatCommands, MagicCommands,
// NpcCommands) y desde WorldNpcs, no solo desde Npc.cpp.
struct NpcTemplate {
    NpcId id;
    std::string name;
    uint16_t max_hp;
    uint16_t dmg_min;
    uint16_t dmg_max;
    uint16_t defense_min;
    uint16_t defense_max;
    uint16_t attack_range;
    uint16_t move_cooldown;
    uint16_t attack_cooldown;
    uint32_t exp_reward;
    uint32_t gold_min;
    uint32_t gold_max;
    std::vector<uint8_t> drop_table;
    bool is_service = false;
};

// Instancia viva de un NPC en el mundo.
struct NpcData {
    uint16_t entity_id;
    NpcId type;
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t direction = 0;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t move_timer = 0;
    uint16_t attack_timer = 0;
    uint8_t zone_id = 255;  // 255 = sin zona (spawn manual)
};
