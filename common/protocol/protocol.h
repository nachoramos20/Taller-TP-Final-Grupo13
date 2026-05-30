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

    // Servidor → Cliente
    LOGIN_OK     = 0x10,
    LOGIN_ERROR  = 0x11,
    SNAPSHOT     = 0xFF,
    MAPA         = 0xFE,
};

enum class MoveDirection : uint8_t {
    NORTH = 0,
    SOUTH = 1,
    EAST  = 2,
    WEST  = 3,
};

enum class EntityType : uint8_t {
    PLAYER     = 0,
    NPC        = 1,
    ITEM_FLOOR = 2,
};

enum class Race : uint8_t {
    HUMAN  = 0,
    ELF    = 1,
    DWARF  = 2,
    GNOME  = 3,
};

enum class Class : uint8_t {
    MAGE    = 0,
    CLERIC  = 1,
    PALADIN = 2,
    WARRIOR = 3,
};

enum class EquipSlot : uint8_t {
    WEAPON  = 0,
    ARMOR   = 1,
    HELMET  = 2,
    SHIELD  = 3,
};