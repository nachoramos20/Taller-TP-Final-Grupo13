#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "../../common/socket.h"
#include "../game/Commands.h"


class ServerProtocol {
public:
    explicit ServerProtocol(Socket&& socket);

    std::shared_ptr<ServerCommand> receive_command(uint16_t client_id);
    void send_snapshot(const SnapshotDTO& snap);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
    ServerProtocol(ServerProtocol&&) = default;
    ServerProtocol& operator=(ServerProtocol&&) = default;

    virtual ~ServerProtocol() = default;

    void shutdown(int how);

private:
    std::shared_ptr<LoginCommand> receive_login_command(uint16_t client_id);
    std::shared_ptr<MoveCommand> receive_move_command(uint16_t client_id);
    void send_uint8(uint8_t value);
    void send_uint16(uint16_t value);
    void send_uint32(uint32_t value);
    void send_str8(const std::string& s);

    Socket socket;

};
