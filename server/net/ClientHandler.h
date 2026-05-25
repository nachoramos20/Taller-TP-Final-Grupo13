#pragma once

#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/protocol/protocol.h"
#include "../../common/protocol/dtos.h"
#include "ServerReceiverThread.h"

#include <atomic>
#include <cstdint>
#include <memory>

class ClientHandler {
public:
    ClientHandler(uint16_t client_id,
                  Socket&& socket,
                  Queue<ServerCommand>& command_queue)
        : _client_id(client_id),
          _socket(std::move(socket)),
          _alive(true),
          _receiver(_client_id, _socket, command_queue, _alive) {}

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
    uint16_t                _client_id;
    Socket                  _socket;
    std::atomic<bool>       _alive;
    Queue<SnapshotDTO>      _snapshot_queue;
    ServerReceiverThread    _receiver;
};