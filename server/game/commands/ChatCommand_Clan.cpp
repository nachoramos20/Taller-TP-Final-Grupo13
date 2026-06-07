#include "Commands.h"

void ChatCommand::handle_fundar_clan(World& world, const std::string& clan_name) {
    if (clan_name.empty()) {
        world.push_message(client_id, 0, "Uso: /fundar-clan <nombre>");
        return;
    }
    world.clan_found(client_id, clan_name);
}

void ChatCommand::handle_unirse(World& world, const std::string& clan_name) {
    if (clan_name.empty()) {
        world.push_message(client_id, 0, "Uso: /unirse <nombre_clan>");
        return;
    }
    world.clan_join_request(client_id, clan_name);
}

void ChatCommand::handle_revisar_clan(World& world) {
    std::string info = world.clan_review(client_id);
    world.push_message(client_id, 0, info);
}

void ChatCommand::handle_clan_aceptar(World& world, const std::string& nick) {
    world.clan_accept(client_id, nick);
}

void ChatCommand::handle_clan_rechazar(World& world, const std::string& nick) {
    world.clan_reject(client_id, nick);
}

void ChatCommand::handle_clan_ban(World& world, const std::string& nick) {
    world.clan_ban(client_id, nick);
}

void ChatCommand::handle_clan_kick(World& world, const std::string& nick) {
    world.clan_kick(client_id, nick);
}

void ChatCommand::handle_dejar_clan(World& world) {
    world.clan_leave(client_id);
}
