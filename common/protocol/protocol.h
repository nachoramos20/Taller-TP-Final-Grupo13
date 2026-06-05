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

// === IDs de items (sprite_id en EntityDTO cuando es ITEM_FLOOR / inventario) ===
enum class ItemId : uint8_t {
    NONE              = 0,
    // Armas c/c
    SWORD             = 1,
    AXE               = 2,
    HAMMER            = 3,
    // Armas a distancia
    SIMPLE_BOW        = 4,
    COMPOUND_BOW      = 5,
    ELVEN_FLUTE       = 6,
    GEMMED_STAFF      = 7,
    // Armaduras
    LEATHER_ARMOR     = 10,
    PLATE_ARMOR       = 11,
    // Cascos
    HOOD              = 20,
    IRON_HELMET       = 21,
    MAGIC_HAT         = 22,
    // Escudos
    TURTLE_SHIELD     = 30,
    IRON_SHIELD       = 31,
    // Pociones
    HEALTH_POTION     = 40,
    MANA_POTION       = 41,
    // Oro (en el suelo se modela como item especial)
    GOLD_PILE         = 50,
};

enum class NpcId : uint8_t {
    GOBLIN   = 1,
    SKELETON = 2,
    ZOMBIE   = 3,
    SPIDER   = 4,
    ORC      = 5,
    GOLEM    = 6,
};
