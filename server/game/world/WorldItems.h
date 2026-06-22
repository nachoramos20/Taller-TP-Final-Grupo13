#ifndef WORLD_ITEMS_H
#define WORLD_ITEMS_H

#include <cstdint>
#include <vector>

#include "../FloorItem.h"
#include "../PlayerData.h"
#include "IdAllocator.h"

class WorldItems {
private:
    std::vector<FloorItem> items;
    IdAllocator& id_alloc;
public:
    explicit WorldItems(IdAllocator& alloc) : id_alloc(alloc) {}

    const std::vector<FloorItem>& all() const { return items; }

    void add(uint8_t item_id, uint16_t x, uint16_t y,
             uint32_t gold = 0, uint32_t spawn_tick = 0);
    uint8_t pick(uint16_t x, uint16_t y, uint32_t& gold_out);

    // Elimina manchas de sangre que ya expiraron
    void cleanup_expired(uint32_t current_tick);

    // Elimina todos los items del suelo dentro de un rectángulo.
    void remove_in_zone(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    // Droppea oro excedente + inventario al morir el jugador.
    void drop_player_loot(PlayerData& dead);
};

#endif