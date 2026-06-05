#include "Commands.h"

MoveCommand::MoveCommand(uint16_t client_id, uint16_t new_x, uint16_t new_y)
    : client_id(client_id), new_x(new_x), new_y(new_y) {}

void MoveCommand::execute(Game &game) {
    game.move_player(client_id, new_x, new_y);
}

LoginCommand::LoginCommand(PlayerData player_data)
    : player_data(player_data) {}

void LoginCommand::execute(Game &game) {
    game.add_player(player_data);
}

const char* LoginCommand::get_username() const {
    return player_data.username;
}