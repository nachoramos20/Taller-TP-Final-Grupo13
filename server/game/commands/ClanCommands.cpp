#include "ClanCommands.h"

ClanCommands::ClanCommands(uint16_t client_id): client_id_(client_id) {}

void ClanCommands::found(World& world, const std::string& clan_name) {
    if (clan_name.empty()) {
        world.push_message(client_id_, 0, "Uso: /fundar-clan <nombre>");
        return;
    }
    world.clan_found(client_id_, clan_name);
}

void ClanCommands::join_request(World& world, const std::string& clan_name) {
    if (clan_name.empty()) {
        world.push_message(client_id_, 0, "Uso: /unirse <nombre_clan>");
        return;
    }
    world.clan_join_request(client_id_, clan_name);
}

void ClanCommands::review(World& world) {
    std::string info = world.clan_review(client_id_);
    world.push_message(client_id_, 0, info);
}

void ClanCommands::accept(World& world, const std::string& nick) {
    world.clan_accept(client_id_, nick);
}

void ClanCommands::reject(World& world, const std::string& nick) {
    world.clan_reject(client_id_, nick);
}

void ClanCommands::ban(World& world, const std::string& nick) { world.clan_ban(client_id_, nick); }

void ClanCommands::kick(World& world, const std::string& nick) {
    world.clan_kick(client_id_, nick);
}

void ClanCommands::leave(World& world) { world.clan_leave(client_id_); }
