#include "WorldItems.h"
#include "../Items.h"
#include <algorithm>
#include <cmath>

// Cantidad de variantes visuales por item_id
static uint8_t sprite_variant_count(uint8_t item_id) {
    switch (item_id) {
        default: return 1;
    }
}

void WorldItems::add(uint8_t item_id, uint16_t x, uint16_t y,
                     uint32_t gold, uint32_t spawn_tick) {
    FloorItem fi{};
    fi.entity_id      = id_alloc.allocate();
    fi.item_id        = item_id;
    fi.sprite_variant = static_cast<uint8_t>(
        rand() % sprite_variant_count(item_id));
    fi.pos_x          = x;
    fi.pos_y          = y;
    fi.gold_amount    = gold;
    fi.spawn_tick     = spawn_tick;
    items.push_back(fi);
}

uint8_t WorldItems::pick(uint16_t x, uint16_t y, uint32_t& gold_out) {
    gold_out = 0;
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->pos_x == x && it->pos_y == y) {
            // La sangre no es recogible
            if (it->item_id == static_cast<uint8_t>(ItemId::BLOOD_STAIN))
                continue;
            uint8_t id = it->item_id;
            gold_out   = it->gold_amount;
            items.erase(it);
            return id;
        }
    }
    return 0;
}

void WorldItems::cleanup_expired(uint32_t current_tick) {
    items.erase(
        std::remove_if(items.begin(), items.end(),
            [&](const FloorItem& fi) {
                return fi.item_id == static_cast<uint8_t>(ItemId::BLOOD_STAIN)
                    && current_tick - fi.spawn_tick >= BLOOD_STAIN_DURATION_TICKS;
            }),
        items.end());
}

void WorldItems::drop_player_loot(PlayerData& dead) {
    if (dead.gold > 0) {
        add(static_cast<uint8_t>(ItemId::GOLD_PILE), dead.pos_x, dead.pos_y, dead.gold);
        dead.gold = 0;
    }
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (dead.inventory[i] != 0) {
            add(dead.inventory[i], dead.pos_x, dead.pos_y, 0);
            dead.inventory[i] = 0;
        }
    }
    dead.equipped_weapon = 0xFF;
    dead.equipped_armor  = 0xFF;
    dead.equipped_helmet = 0xFF;
    dead.equipped_shield = 0xFF;
}
