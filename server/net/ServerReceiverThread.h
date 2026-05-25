#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Deserializer.h"
#include "../../common/protocol/Serializer.h"
#include "../../common/protocol/protocol.h"

#include <atomic>
#include <cstdint>
#include <string>

struct ServerCommand {
    uint16_t    client_id  = 0;
    MsgType     type       = MsgType::LOGOUT;
    uint16_t    pos_x      = 0;
    uint16_t    pos_y      = 0;
    uint16_t    target_id  = 0;
    uint8_t     slot       = 0;
    uint8_t     race       = 0;
    uint8_t     cls        = 0;
    std::string text;
};

class ServerReceiverThread : public Thread {
public:
    ServerReceiverThread(uint16_t client_id,
                         Socket& socket,
                         Queue<ServerCommand>& command_queue,
                         std::atomic<bool>& client_alive);

    void run() override;
    void stop() override;

private:
    ServerCommand deserialize_command(MsgType type);

    uint16_t              _client_id;
    Deserializer          _deserializer;
    Serializer            _serializer;
    Queue<ServerCommand>& _command_queue;
    std::atomic<bool>&    _client_alive;
};