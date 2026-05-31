#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <string>
#include <cstdint>

struct PlayerData {
    std::string username;
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

};

#endif // PLAYERDATA_H