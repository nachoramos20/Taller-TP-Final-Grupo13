#pragma once

#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/protocol/protocol.h"
#include "../../common/protocol/dtos.h"

#include <atomic>
#include <cstdint>

class ClientHandler {
public:
    explicit ClientHandler(Socket&& socket)
        : _socket(std::move(socket)), _alive(true) {}

    void start();
    void stop();
    void join();

    bool is_alive() const { return _alive; }

    Queue<SnapshotDTO>& snapshot_queue() { return _snapshot_queue; }

    ~ClientHandler() {}

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = default;

private:
    Socket             _socket;
    Queue<SnapshotDTO> _snapshot_queue;
    std::atomic<bool>  _alive;
};