#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct EntityDTO {
    uint16_t entity_id;
    uint8_t  entity_type;
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t  direction;
    uint8_t  sprite_id;
    uint8_t  is_ghost;
    uint8_t  hp_pct;
};

struct ChatMessageDTO {
    uint8_t     msg_type;
    std::string text;
};

struct SnapshotDTO {
    uint32_t tick;
    uint16_t self_entity_id;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t mp;
    uint16_t max_mp;
    uint32_t exp;
    uint8_t  level;
    uint32_t gold;
    uint8_t  is_ghost;
    uint8_t  meditating;

    static constexpr int INVENTORY_SIZE = 16;
    uint8_t inventory[INVENTORY_SIZE];
    uint8_t equipped_wpn;
    uint8_t equipped_arm;
    uint8_t equipped_helm;
    uint8_t equipped_shld;

    std::shared_ptr<std::vector<EntityDTO>>      entities;
    std::shared_ptr<std::vector<ChatMessageDTO>> messages;
};