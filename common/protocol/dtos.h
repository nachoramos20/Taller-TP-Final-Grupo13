#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "../constants.h"

// DTOs del SNAPSHOT periódico (ver ServerProtocol::send_snapshot /
// ClientProtocol::recv_snapshot), el mensaje que el server difunde a cada
// cliente en cada tick con su propio estado y el de las entidades visibles.

// Una entidad visible para el jugador (otro player, NPC o item en el piso).
struct EntityDTO {
    uint16_t entity_id;
    uint8_t  entity_type;
    std::string username;
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t  direction;
    uint8_t  sprite_id;
    uint8_t  is_ghost;
    uint8_t  hp_pct;

    // Equipo visible (ItemId, 0 = nada equipado en ese slot). Solo se usa
    // para entity_type == PLAYER; NPCs e items en el piso van en 0.
    uint8_t  equipped_weapon = 0;
    uint8_t  equipped_armor  = 0;
    uint8_t  equipped_helmet = 0;
    uint8_t  equipped_shield = 0;

    // Nivel del jugador (0 para NPC/item en el piso). El cliente lo necesita
    // para poder replicar localmente el chequeo de fair-play
    // (Equations::is_pvp_allowed) antes de spawnear VFX de ataque/hechizo.
    uint8_t  level = 0;
};

// Un mensaje de chat/sistema para mostrar en el ChatWidget del cliente.
struct ChatMessageDTO {
    uint8_t     msg_type;
    std::string text;
};

// Evento de VFX de ataque/hechizo exitoso (no rechazado por el server),
// para que todos los clientes vean el proyectil/hechizo de los demás
// jugadores y no solo el propio (que ya lo spawnea localmente y de forma
// optimista al clickear, antes de que llegue ningún snapshot).
struct SpellEventDTO {
    uint16_t caster_id;
    uint8_t  spell_id;    // 0 = ataque físico/proyectil, >0 = hechizo (SpellId)
    uint16_t target_x;
    uint16_t target_y;
    bool     is_magic_projectile;
};

// Estado completo del jugador propio (stats, inventario, equipo) más las
// entidades visibles a su alrededor y los mensajes de chat pendientes.
struct SnapshotDTO {
    uint32_t tick;
    uint16_t self_entity_id;
    uint16_t hp;
    uint16_t max_hp;
    uint16_t mp;
    uint16_t max_mp;
    uint32_t exp;
    uint8_t  level;
    uint8_t  character_class;
    uint32_t gold;
    uint8_t  is_ghost;
    uint8_t  meditating;

    static constexpr int INVENTORY_SIZE = PROTOCOL_INVENTORY_SIZE;
    uint8_t inventory[INVENTORY_SIZE];
    uint8_t equipped_weapon;
    uint8_t equipped_armor;
    uint8_t equipped_helmet;
    uint8_t equipped_shield;

    std::shared_ptr<std::vector<EntityDTO>>      entities;
    std::shared_ptr<std::vector<ChatMessageDTO>> messages;
    std::shared_ptr<std::vector<SpellEventDTO>>  spell_events;
};
