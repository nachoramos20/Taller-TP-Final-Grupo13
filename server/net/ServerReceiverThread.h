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
#include "ServerProtocol.h"


class ServerReceiverThread : public Thread {
public:
    ServerReceiverThread(uint16_t client_id,
                         Queue<std::shared_ptr<ServerCommand>>& command_queue,
                         std::atomic<bool>& client_alive, ServerProtocol& server_protocol);

    void run() override;
    void stop() override;

private:

    uint16_t              client_id;
    ServerProtocol&        server_protocol;
    Queue<std::shared_ptr<ServerCommand>>& command_queue;
    std::atomic<bool>&    client_alive;
};