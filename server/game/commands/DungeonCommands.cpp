#include "DungeonCommands.h"
#include "../config/MapConstants.h"
#include "../entities/PlayerData.h"
#include <algorithm>
#include <cstdlib>

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
    int dx = std::min(std::abs(player_x - static_cast<int>(MAP_DUNGEON_DOOR_X1)),
                      std::abs(player_x - static_cast<int>(MAP_DUNGEON_DOOR_X2)));
    int dy = std::abs(player_y - static_cast<int>(MAP_DUNGEON_DOOR_Y));
    if (dx > MAP_DUNGEON_DOOR_RADIUS || dy > MAP_DUNGEON_DOOR_RADIUS) {
        world.push_message(client_id_, 0, "Debes estar en la puerta del cementerio para entrar a la mazmorra.");
        return;
    }

    Mazmorra* mazmorra = world.get_dungeon_at(MAP_DUNGEON_SPAWN_X, MAP_DUNGEON_SPAWN_Y);
    if (!mazmorra) {
        world.push_message(client_id_, 0, "Mazmorra no encontrada");
        return;
    }
    if (mazmorra->in_mazmorra(player->pos_x, player->pos_y)) {
        world.push_message(client_id_, 0, "Ya estás dentro de la mazmorra.");
        return;
    }

    mazmorra->player_entered();

    world.tp_player(client_id_, MAP_DUNGEON_SPAWN_X, MAP_DUNGEON_SPAWN_Y);
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

    world.tp_player(client_id_, MAP_DUNGEON_DOOR_X1, MAP_DUNGEON_DOOR_Y);

    world.push_message(client_id_, 0, "Has salido de la mazmorra.");
}

void DungeonCommands::info(World& world) {
    world.push_message(client_id_, 0,
        "La entrada a la mazmorra esta en la puerta de la casa del cementerio. Parate ahi y usa /entrar-mazmorra.");
}
