#include "WorldItems.h"
#include "../Items.h"
#include <algorithm>
#include <cmath>

// Cantidad de variantes visuales por item_id
static uint8_t sprite_variant_count(uint8_t item_id) {
    switch (item_id) {
        case  1: return 2;  // espada: espada_comun, espada_oscura
        case  2: return 2;  // hacha: hacha_hierro, hacha_epica
        case  3: return 3;  // martillo: martillo_comun, martillo_epico, martillo_legendario
        case  4: return 2;  // arco simple: arco_simple_madera, arco_simple_amatista
        case  5: return 2;  // arco compuesto: arco_compuesto_oro, arco_compuesto_infernal
        case  6: return 1;  // flauta
        case  7: return 3;  // báculo: baculo_esmeralda, baculo_egipcio, baculo_esqueletico
        case  8: return 3;  // vara: vara_fresno, vara_cuarzo, vara_muerdago
        case 10: return 4;  // armadura liviana: clerigo_blanco, clerigo_negro, mago_comun, mago_real
        case 11: return 4;  // armadura pesada: guerrero_ejecutor, guerrero_epico, paladin_magico, paladin_real
        case 30: return 1;  // escudo tortuga
        case 31: return 2;  // escudo: escudo_hierro, escudo_boca
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