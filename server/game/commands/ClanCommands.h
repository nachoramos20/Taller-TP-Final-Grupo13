#pragma once

#include <cstdint>
#include <string>

#include "../world/World.h"

// Operaciones de clan (fundar/unirse/revisar/aceptar/rechazar/banear/
// expulsar/dejar) para un cliente. Extraída de ChatCommand, que solo
// delegaba en World::clan_* (a su vez delega en WorldClans) — esto era
// ya una responsabilidad aparte viviendo dentro de la clase ChatCommand.
class ClanCommands {
public:
    explicit ClanCommands(uint16_t client_id);

    void found(World& world, const std::string& clan_name);
    void join_request(World& world, const std::string& clan_name);
    void review(World& world);
    void accept(World& world, const std::string& nick);
    void reject(World& world, const std::string& nick);
    void ban(World& world, const std::string& nick);
    void kick(World& world, const std::string& nick);
    void leave(World& world);

private:
    uint16_t client_id_;
};
