#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

struct PlayerData {
    static constexpr std::size_t USERNAME_MAX_LENGTH = 32;

    char     username[USERNAME_MAX_LENGTH + 1];
    uint16_t entity_id;
    uint8_t  race;
    uint8_t  cls;
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t  direction;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t mp;
    uint16_t max_mp;
    uint32_t exp;
    uint8_t  level;
    uint32_t gold;
    bool     is_ghost;
    static void copy_username(char (&destination)[PlayerData::USERNAME_MAX_LENGTH + 1],
                              const std::string& source) {
        std::memset(destination, 0, sizeof(destination));
        std::memcpy(destination,
                    source.data(),
                    std::min(source.size(), PlayerData::USERNAME_MAX_LENGTH));
    }

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


#endif // PLAYERDATA_H