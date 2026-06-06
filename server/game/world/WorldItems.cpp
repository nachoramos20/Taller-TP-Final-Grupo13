#include "WorldItems.h"
#include "../Items.h"
#include <cmath>

void WorldItems::add(uint8_t item_id, uint16_t x, uint16_t y, uint32_t gold) {
    FloorItem fi{};
    fi.entity_id   = id_alloc.allocate();
    fi.item_id     = item_id;
    fi.pos_x       = x;
    fi.pos_y       = y;
    fi.gold_amount = gold;
    items.push_back(fi);
}

uint8_t WorldItems::pick(uint16_t x, uint16_t y, uint32_t& gold_out) {
    gold_out = 0;
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->pos_x == x && it->pos_y == y) {
            uint8_t id = it->item_id;
            gold_out   = it->gold_amount;
            items.erase(it);
            return id;
        }
    }
    return 0;
}

void WorldItems::drop_player_loot(PlayerData& dead) {
    uint32_t oro_max = static_cast<uint32_t>(
        100.0 * std::pow(static_cast<double>(dead.level), 1.1));
    if (dead.gold > oro_max) {
        uint32_t excess = dead.gold - oro_max;
        dead.gold = oro_max;
        add(static_cast<uint8_t>(ItemId::GOLD_PILE), dead.pos_x, dead.pos_y, excess);
    }
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (dead.inventory[i] != 0) {
            add(dead.inventory[i], dead.pos_x, dead.pos_y, 0);
            dead.inventory[i] = 0;
        }
    }
}
