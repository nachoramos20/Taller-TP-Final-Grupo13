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
    MOVE_ITEM    = 0x12,
    CHEAT        = 0x13,

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

// Payload de MsgType::CHEAT: 1 byte indicando qué cheat se activa/alterna.
// Se disparan con combinaciones de teclas en el cliente (ver InputController).
enum class CheatId : uint8_t {
    INFINITE_HP    = 1,  // Ctrl+Shift+H: alterna vida infinita
    INFINITE_MP    = 2,  // Ctrl+Shift+N: alterna mana infinito
    INSTANT_DEATH  = 3,  // Ctrl+Shift+K: morir instantáneamente
    INSTANT_REVIVE = 4,  // Ctrl+Shift+R: revivir instantáneamente (si es fantasma)
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
    // Armas a distancia físicas (flechas infinitas)
    SIMPLE_BOW        = 4,
    COMPOUND_BOW      = 5,
    // Armas mágicas (consumen mana en ataque básico Y habilitan su hechizo)
    ELVEN_FLUTE       = 6,   // "curar"
    GEMMED_STAFF      = 7,   // "explosion"
    ASH_STICK         = 8,   // "flecha mágica"
    NUDOSO_STAFF      = 9,   // "misil"
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

    MERCHANT = 7,
    BANKER   = 8,
    PRIEST   = 9,
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

inline bool npc_is_service(NpcId id) {
    return id == NpcId::MERCHANT || id == NpcId::BANKER || id == NpcId::PRIEST;
}

// Devuelve true si el item habilita el lanzamiento de hechizos
// (flauta, báculo o vara mágica)
inline bool weapon_enables_spells(uint8_t item_id) {
    return item_id == static_cast<uint8_t>(ItemId::ELVEN_FLUTE)  ||
           item_id == static_cast<uint8_t>(ItemId::GEMMED_STAFF) ||
           item_id == static_cast<uint8_t>(ItemId::ASH_STICK)    ||
           item_id == static_cast<uint8_t>(ItemId::NUDOSO_STAFF);
}

// Devuelve true si el item es un arma mágica que consume mana en ataque básico
inline bool weapon_is_magic(uint8_t item_id) {
    return weapon_enables_spells(item_id);
}

// Devuelve true si el item es un arma a distancia (arco o mágica), para
// decidir si el ataque básico dispara una animación de proyectil.
inline bool weapon_is_ranged(uint8_t item_id) {
    return item_id == static_cast<uint8_t>(ItemId::SIMPLE_BOW)   ||
           item_id == static_cast<uint8_t>(ItemId::COMPOUND_BOW) ||
           weapon_is_magic(item_id);
}

// Rango de ataque del arma en tiles (client-side, espejado de items.toml).
// Usado para validación visual antes de spawnear proyectiles/hechizos.
inline int weapon_client_range(uint8_t item_id) {
    switch (static_cast<ItemId>(item_id)) {
        case ItemId::SIMPLE_BOW:    return 6;
        case ItemId::COMPOUND_BOW:  return 8;
        case ItemId::ELVEN_FLUTE:   return 6;
        case ItemId::ASH_STICK:     return 5;
        case ItemId::NUDOSO_STAFF:  return 6;
        case ItemId::GEMMED_STAFF:  return 6;
        default:                    return 1;
    }
}

// Costo de maná del ataque básico con armas mágicas (client-side, espejado
// de items.toml). Usado para no spawnear el proyectil si claramente no hay
// maná suficiente: antes solo se chequeaba "mp > 0", lo que dejaba disparar
// con menos maná del que la propia arma cuesta.
inline uint16_t weapon_client_mana_cost(uint8_t item_id) {
    switch (static_cast<ItemId>(item_id)) {
        case ItemId::ASH_STICK:     return 5;
        case ItemId::ELVEN_FLUTE:   return 100;
        case ItemId::NUDOSO_STAFF:  return 15;
        case ItemId::GEMMED_STAFF:  return 30;
        default:                    return 0;
    }
}
