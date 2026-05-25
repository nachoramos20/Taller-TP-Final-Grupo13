#pragma once

#include "../../common/socket.h"
#include "../../common/queue.h"
#include "../../common/protocol/dtos.h"
#include "ServerReceiverThread.h"
#include "ServerSenderThread.h"

#include <atomic>
#include <cstdint>

class ClientHandler {
public:
    ClientHandler(uint16_t client_id,
                  Socket&& socket,
                  Queue<std::shared_ptr<ServerCommand>>& command_queue);

    void start();
    void stop();
    void join();

    bool is_alive() const { return _alive; }
    uint16_t client_id() const { return _client_id; }
    Queue<SnapshotDTO>& snapshot_queue() { return _snapshot_queue; }

    ~ClientHandler() {}

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

private:
    uint16_t             _client_id;
    Socket               _socket;
    std::atomic<bool>    _alive;
    Queue<SnapshotDTO>   _snapshot_queue;
    ServerReceiverThread _receiver;
    ServerSenderThread   _sender;
};