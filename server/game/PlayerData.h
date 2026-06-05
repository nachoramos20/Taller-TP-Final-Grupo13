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

};


#endif // PLAYERDATA_H