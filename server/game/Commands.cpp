#include "Commands.h"

MoveCommand::MoveCommand(uint16_t client_id, uint16_t new_x, uint16_t new_y)
    : client_id(client_id), new_x(new_x), new_y(new_y) {}

void MoveCommand::execute(Game &game) {
    game.move_player(client_id, new_x, new_y);
}

LoginCommand::LoginCommand(uint16_t client_id, const std::string& username)
    : client_id(client_id), username(username) {}

void LoginCommand::execute(Game &game) {
    game.add_player(this->client_id, this->username);
}