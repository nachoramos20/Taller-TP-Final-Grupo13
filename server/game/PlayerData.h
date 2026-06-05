#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <string>
#include <cstdint>
#include <array>
#include "../../common/protocol/protocol.h"

struct PlayerData {
    std::string username;
    uint16_t entity_id   = 0;
    uint8_t  race        = 0;
    uint8_t  cls         = 0;
    uint16_t pos_x       = 0;
    uint16_t pos_y       = 0;
    uint8_t  direction   = 0;

    uint16_t hp          = 0;
    uint16_t max_hp      = 0;
    uint16_t mp          = 0;
    uint16_t max_mp      = 0;
    uint32_t exp         = 0;
    uint8_t  level       = 1;
    uint32_t gold        = 0;
    bool     is_ghost    = false;
    bool     meditating  = false;

    uint16_t strength    = 18;
    uint16_t agility     = 18;
    uint16_t intelligence= 18;
    uint16_t constitution= 18;

    static constexpr int INVENTORY_SIZE = 16;
    std::array<uint8_t, INVENTORY_SIZE> inventory{};

    uint8_t equipped_weapon = 0;
    uint8_t equipped_armor  = 0;
    uint8_t equipped_helmet = 0;
    uint8_t equipped_shield = 0;

    uint16_t attack_cooldown = 0;
};

#endif
