#ifndef NPC_H
#define NPC_H

#include <cstdint>
#include <string>
#include <vector>
#include "../../common/protocol/protocol.h"

struct NpcTemplate {
    NpcId    id;
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
};

struct NpcData {
    uint16_t entity_id;
    NpcId    type;
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t  direction = 0;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t move_timer = 0;
    uint16_t attack_timer = 0;
    uint8_t  zone_id = 255;   // 255 = sin zona (spawn manual)
};

namespace Npcs {
    const NpcTemplate& tpl(NpcId id);
    const std::vector<NpcId>& all_ids();
}

#endif
