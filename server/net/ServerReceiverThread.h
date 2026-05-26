#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Deserializer.h"
#include "../../common/protocol/Serializer.h"
#include "../../common/protocol/protocol.h"
#include "../game/Commands.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <memory>


class ServerReceiverThread : public Thread {
public:
    ServerReceiverThread(uint16_t client_id,
                         Socket& socket,
                         Queue<std::shared_ptr<ServerCommand>>& command_queue,
                         std::atomic<bool>& client_alive);

    void run() override;
    void stop() override;

private:
    std::shared_ptr<ServerCommand> deserialize_command(MsgType type);

    uint16_t              _client_id;
    Deserializer          _deserializer;
    Serializer            _serializer;
    Queue<std::shared_ptr<ServerCommand>>& _command_queue;
    std::atomic<bool>&    _client_alive;
};