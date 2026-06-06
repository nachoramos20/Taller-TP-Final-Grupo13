#include "Commands.h"
#include "../Items.h"
#include <utility>

LoginCommand::LoginCommand(PlayerData p) : player_data(std::move(p)) {}

void LoginCommand::execute(World& world) {
    player_data.inventory[0]    = static_cast<uint8_t>(ItemId::SWORD);
    player_data.equipped_weapon = static_cast<uint8_t>(ItemId::SWORD);
    world.add_player(player_data);
}

const char* LoginCommand::get_username() const {
    return player_data.username;
}

LogoutCommand::LogoutCommand(uint16_t c) : client_id(c) {}

void LogoutCommand::execute(World& world) {
    world.remove_player(client_id);
}
