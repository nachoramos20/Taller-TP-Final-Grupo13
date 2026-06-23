#include "DungeonCommands.h"
#include "../entities/PlayerData.h"
#include <algorithm>
#include <cstdlib>

// Puerta del cementerio: ocupa dos tiles de ancho en X (60 y 61) a la
// altura y=62. Es el único punto de entrada a la mazmorra (ver enter())
// y lo que describe info().
namespace {
    constexpr uint16_t CEMENTERIO_PUERTA_X1 = 60;
    constexpr uint16_t CEMENTERIO_PUERTA_X2 = 61;
    constexpr uint16_t CEMENTERIO_PUERTA_Y  = 62;
    constexpr uint16_t CEMENTERIO_PUERTA_RADIO = 2;
}

DungeonCommands::DungeonCommands(uint16_t client_id) : client_id_(client_id) {}

void DungeonCommands::enter(World& world) {
    PlayerData* player = world.get_player_mutable(client_id_);
    if (!player) {
        return;
    }

    if (player->is_ghost) {
        world.push_message(client_id_, 0, "No puedes entrar a la mazmorra estando muerto.");
        return;
    }

    int player_x = static_cast<int>(player->pos_x);
    int player_y = static_cast<int>(player->pos_y);
    int dx = std::min(std::abs(player_x - static_cast<int>(CEMENTERIO_PUERTA_X1)),
                      std::abs(player_x - static_cast<int>(CEMENTERIO_PUERTA_X2)));
    int dy = std::abs(player_y - static_cast<int>(CEMENTERIO_PUERTA_Y));
    if (dx > CEMENTERIO_PUERTA_RADIO || dy > CEMENTERIO_PUERTA_RADIO) {
        world.push_message(client_id_, 0, "Debes estar en la puerta del cementerio para entrar a la mazmorra.");
        return;
    }

    uint16_t spawn_mazmorra_x = 114;
    uint16_t spawn_mazmorra_y = 79;

    Mazmorra* mazmorra = world.get_dungeon_at(spawn_mazmorra_x, spawn_mazmorra_y);
    if (!mazmorra) {
        world.push_message(client_id_, 0, "Mazmorra no encontrada");
        return;
    }
    if (mazmorra->in_mazmorra(player->pos_x, player->pos_y)) {
        world.push_message(client_id_, 0, "Ya estás dentro de la mazmorra.");
        return;
    }

    mazmorra->player_entered();

    world.tp_player(client_id_, spawn_mazmorra_x, spawn_mazmorra_y);
    world.push_message(client_id_, 0, "Has entrado a la mazmorra.");
}

void DungeonCommands::leave(World& world) {
    PlayerData* player = world.get_player_mutable(client_id_);
    if (!player) return;

    Mazmorra* mazmorra = world.get_dungeon_at(player->pos_x, player->pos_y);
    if (!mazmorra) {
        world.push_message(client_id_, 0, "No estás dentro de una mazmorra.");
        return;
    }

    mazmorra->player_left();

    uint16_t portal_spawn_x = CEMENTERIO_PUERTA_X1;
    uint16_t portal_spawn_y = CEMENTERIO_PUERTA_Y;
    world.tp_player(client_id_, portal_spawn_x, portal_spawn_y);

    world.push_message(client_id_, 0, "Has salido de la mazmorra.");
}

void DungeonCommands::info(World& world) {
    world.push_message(client_id_, 0,
        "La entrada a la mazmorra esta en la puerta de la casa del cementerio. Parate ahi y usa /entrar-mazmorra.");
}
