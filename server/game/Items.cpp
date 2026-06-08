#include "Items.h"
#include <unordered_map>
#include <stdexcept>

static const std::unordered_map<uint8_t, ItemDef>& catalog() {
    static const std::unordered_map<uint8_t, ItemDef> c = {
        // id, kind, name, min_dmg, max_dmg, mana_cost, range_tiles

        // Armas cuerpo a cuerpo (sin mana, rango 1)
        {(uint8_t)ItemId::SWORD,         {ItemId::SWORD,         ItemKind::WEAPON_MELEE,  "Espada",              4, 12,  0, 1}},
        {(uint8_t)ItemId::AXE,           {ItemId::AXE,           ItemKind::WEAPON_MELEE,  "Hacha",               5, 14,  0, 1}},
        {(uint8_t)ItemId::HAMMER,        {ItemId::HAMMER,        ItemKind::WEAPON_MELEE,  "Martillo",            6, 16,  0, 1}},

        // Armas a distancia físicas (sin mana)
        {(uint8_t)ItemId::SIMPLE_BOW,    {ItemId::SIMPLE_BOW,    ItemKind::WEAPON_RANGED, "Arco Simple",         3, 10,  0, 6}},
        {(uint8_t)ItemId::COMPOUND_BOW,  {ItemId::COMPOUND_BOW,  ItemKind::WEAPON_RANGED, "Arco Compuesto",      5, 14,  0, 8}},

        // Armas mágicas (consumen mana en ataque básico, habilitan hechizos)
        {(uint8_t)ItemId::ELVEN_FLUTE,   {ItemId::ELVEN_FLUTE,   ItemKind::WEAPON_MAGIC,  "Flauta Élfica",       4,  9,  5, 6}},
        {(uint8_t)ItemId::ASH_STICK,     {ItemId::ASH_STICK,     ItemKind::WEAPON_MAGIC,  "Vara Mágica",         6, 13,  8, 5}},
        {(uint8_t)ItemId::GEMMED_STAFF,  {ItemId::GEMMED_STAFF,  ItemKind::WEAPON_MAGIC,  "Báculo Engarzado",    10, 22, 12, 6}},

        // Armaduras
        {(uint8_t)ItemId::LEATHER_ARMOR, {ItemId::LEATHER_ARMOR, ItemKind::ARMOR,  "Armadura de Cuero",   1,  4, 0, 0}},
        {(uint8_t)ItemId::PLATE_ARMOR,   {ItemId::PLATE_ARMOR,   ItemKind::ARMOR,  "Armadura de Placas",  4,  9, 0, 0}},

        // Cascos
        {(uint8_t)ItemId::HOOD,          {ItemId::HOOD,          ItemKind::HELMET, "Capucha",             0,  2, 0, 0}},
        {(uint8_t)ItemId::IRON_HELMET,   {ItemId::IRON_HELMET,   ItemKind::HELMET, "Casco de Hierro",     2,  5, 0, 0}},
        {(uint8_t)ItemId::MAGIC_HAT,     {ItemId::MAGIC_HAT,     ItemKind::HELMET, "Sombrero Mágico",     1,  3, 0, 0}},

        // Escudos
        {(uint8_t)ItemId::TURTLE_SHIELD, {ItemId::TURTLE_SHIELD, ItemKind::SHIELD, "Escudo Tortuga",      1,  3, 0, 0}},
        {(uint8_t)ItemId::IRON_SHIELD,   {ItemId::IRON_SHIELD,   ItemKind::SHIELD, "Escudo de Hierro",    3,  7, 0, 0}},

        // Pociones
        {(uint8_t)ItemId::HEALTH_POTION, {ItemId::HEALTH_POTION, ItemKind::POTION, "Poción de Vida",     30, 30, 0, 0}},
        {(uint8_t)ItemId::MANA_POTION,   {ItemId::MANA_POTION,   ItemKind::POTION, "Poción de Maná",     30, 30, 0, 0}},

        // Oro
        {(uint8_t)ItemId::GOLD_PILE,     {ItemId::GOLD_PILE,     ItemKind::GOLD,   "Oro",                 0,  0, 0, 0}},
    };
    return c;
}

namespace Items {

const ItemDef& get(ItemId id) {
    auto it = catalog().find(static_cast<uint8_t>(id));
    if (it == catalog().end()) throw std::runtime_error("ItemId desconocido");
    return it->second;
}

bool exists(ItemId id) {
    return catalog().count(static_cast<uint8_t>(id)) > 0;
}

EquipSlot equip_slot_for(ItemKind kind) {
    switch (kind) {
        case ItemKind::WEAPON_MELEE:
        case ItemKind::WEAPON_RANGED:
        case ItemKind::WEAPON_MAGIC:  return EquipSlot::WEAPON;
        case ItemKind::ARMOR:         return EquipSlot::ARMOR;
        case ItemKind::HELMET:        return EquipSlot::HELMET;
        case ItemKind::SHIELD:        return EquipSlot::SHIELD;
        default: throw std::runtime_error("Item no equipable");
    }
}

} // namespace Items
