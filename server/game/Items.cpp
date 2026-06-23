#include "Items.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

#include "config/GameConfig.h"

namespace Items {

const ItemDef& get(ItemId id) {
    // Convertimos ItemConfig → ItemDef on-demand con un cache estático.
    // La primera vez que se pide un ítem, se construye el ItemDef y se cachea.
    static std::unordered_map<uint8_t, ItemDef> cache;
    uint8_t raw = static_cast<uint8_t>(id);

    auto it = cache.find(raw);
    if (it != cache.end())
        return it->second;

    const GameConfig& cfg = GameConfig::get();
    if (!cfg.item_exists(id))
        throw std::runtime_error("ItemId desconocido");

    const ItemConfig& ic = cfg.item(id);
    ItemDef def;
    def.id = ic.id;
    def.kind = ic.kind;
    def.name = ic.name;
    def.min_value = ic.min_value;
    def.max_value = ic.max_value;
    def.mana_cost = ic.mana_cost;
    def.range_tiles = ic.range_tiles;

    cache[raw] = def;
    return cache[raw];
}

bool exists(ItemId id) { return GameConfig::get().item_exists(id); }

EquipSlot equip_slot_for(ItemKind kind) { return GameConfig::get().equip_slot_for(kind); }

bool name_equals_ci(const std::string& a, const std::string& b) {
    if (a.size() != b.size())
        return false;
    return std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
        return std::tolower(static_cast<unsigned char>(x)) ==
               std::tolower(static_cast<unsigned char>(y));
    });
}

}  // namespace Items
