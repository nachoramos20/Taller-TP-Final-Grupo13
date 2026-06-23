#ifndef FLOOR_ITEM_H
#define FLOOR_ITEM_H

#include <cstdint>

static constexpr uint32_t BLOOD_STAIN_DURATION_TICKS = 300;  // 5s a 60 ticks/s

// Un item (o pila de oro) tirado en el piso, recogible con /tomar.
struct FloorItem {
    uint16_t entity_id;
    uint8_t item_id;
    uint8_t sprite_variant = 0;
    uint16_t pos_x;
    uint16_t pos_y;
    uint32_t gold_amount;
    uint32_t spawn_tick = 0;
};

#endif
