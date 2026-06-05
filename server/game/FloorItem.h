#ifndef FLOOR_ITEM_H
#define FLOOR_ITEM_H

#include <cstdint>

struct FloorItem {
    uint16_t entity_id;
    uint8_t  item_id;
    uint16_t pos_x;
    uint16_t pos_y;
    uint32_t gold_amount;
};

#endif
