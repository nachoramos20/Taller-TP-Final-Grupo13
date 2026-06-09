#pragma once

#include <cstdint>

enum class MsgType : uint8_t {
    // Cliente → Servidor
    LOGIN        = 0x01,
    REGISTER     = 0x02,
    MOVE         = 0x03,
    ATTACK       = 0x04,
    CHAT_COMMAND = 0x05,
    EQUIP_ITEM   = 0x06,
    UNEQUIP_ITEM = 0x07,
    DROP_ITEM    = 0x08,
    PICK_ITEM    = 0x09,
    NPC_INTERACT = 0x0A,
    LOGOUT       = 0x0B,
    USE_ITEM     = 0x0C,
    MEDITATE     = 0x0D,
    RESURRECT    = 0x0E,
    CAST_SPELL   = 0x0F,

    // Servidor → Cliente
    LOGIN_OK     = 0x10,
    LOGIN_ERROR  = 0x11,
    SNAPSHOT     = 0xFF,
    MAPA         = 0xFE,
};

enum class MoveDirection : uint8_t {
    NORTH = 0, SOUTH = 1, EAST = 2, WEST = 3,
};

enum class EntityType : uint8_t {
    PLAYER     = 0,
    NPC        = 1,
    ITEM_FLOOR = 2,
};

enum class Race : uint8_t {
    HUMAN = 0, ELF = 1, DWARF = 2, GNOME = 3,
};

enum class Class : uint8_t {
    MAGE = 0, CLERIC = 1, PALADIN = 2, WARRIOR = 3,
};

enum class EquipSlot : uint8_t {
    WEAPON = 0, ARMOR = 1, HELMET = 2, SHIELD = 3,
};

enum class ItemId : uint8_t {
    NONE              = 0,
    // Armas c/c
    SWORD             = 1,
    AXE               = 2,
    HAMMER            = 3,
    DARK_SWORD        = 9,
    EPIC_AXE          = 12,
    EPIC_HAMMER       = 13,
    LEGENDARY_HAMMER  = 14,
    // Armas a distancia físicas
    SIMPLE_BOW        = 4,
    COMPOUND_BOW      = 5,
    AMETHYST_BOW      = 15,
    INFERNAL_BOW      = 16,
    // Armas mágicas  (consumen mana en ataque básico Y habilitan hechizos)
    ELVEN_FLUTE       = 6,
    GEMMED_STAFF      = 7,
    ASH_STICK        = 8,   // vara mágica — nuevo
    EGYPTIAN_STAFF    = 17,
    SKELETAL_STAFF    = 18,
    QUARTZ_STICK      = 19,
    MISTLETOE_STICK   = 23,
    // Armaduras
    LEATHER_ARMOR     = 10,
    PLATE_ARMOR       = 11,
    CLERIC_BLACK_ARMOR = 24,
    MAGE_COMMON_ARMOR = 25,
    MAGE_ROYAL_ARMOR  = 26,
    WARRIOR_EPIC_ARMOR = 27,
    PALADIN_MAGIC_ARMOR = 28,
    PALADIN_ROYAL_ARMOR = 29,
    // Cascos
    HOOD              = 20,
    IRON_HELMET       = 21,
    MAGIC_HAT         = 22,
    // Escudos
    TURTLE_SHIELD     = 30,
    IRON_SHIELD       = 31,
    BOCA_SHIELD       = 32,
    // Pociones
    HEALTH_POTION     = 40,
    MANA_POTION       = 41,
    // Oro
    GOLD_PILE         = 50,
    // Efectos de suelo (no recogibles)
    BLOOD_STAIN       = 60,
};

enum class NpcId : uint8_t {
    GOBLIN = 1, SKELETON = 2, ZOMBIE = 3, SPIDER = 4, ORC = 5, GOLEM = 6,
};

// Hechizos
enum class SpellId : uint8_t {
    NONE                      = 0,
    // Mago
    BURST                     = 1,
    POISON_AREA               = 2,
    SKULL_EXPLOSION           = 3,
    // Clérigo
    ICE_ORB                   = 4,
    GRAVITATIONAL_TORNAD      = 5,
    THUNDERSTORM              = 6,
    // Paladín
    ORB_OF_EMPTINESS          = 7,
    VACUUM_GAP                = 8,
    TORNADO_OF_DARKNESS       = 9,
};

// Devuelve true si el item habilita el lanzamiento de hechizos
// (flauta, báculo o vara mágica)
inline bool weapon_enables_spells(uint8_t item_id) {
    return item_id == static_cast<uint8_t>(ItemId::ELVEN_FLUTE)  ||
           item_id == static_cast<uint8_t>(ItemId::GEMMED_STAFF) ||
           item_id == static_cast<uint8_t>(ItemId::ASH_STICK) ||
           item_id == static_cast<uint8_t>(ItemId::EGYPTIAN_STAFF) ||
           item_id == static_cast<uint8_t>(ItemId::SKELETAL_STAFF) ||
           item_id == static_cast<uint8_t>(ItemId::QUARTZ_STICK) ||
           item_id == static_cast<uint8_t>(ItemId::MISTLETOE_STICK);
}

// Devuelve true si el item es un arma mágica que consume mana en ataque básico
inline bool weapon_is_magic(uint8_t item_id) {
    return weapon_enables_spells(item_id);
}
