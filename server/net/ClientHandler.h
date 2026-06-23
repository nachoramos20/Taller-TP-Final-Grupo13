#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "../../common/protocol/dtos.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../game/PersistenceMonitor.h"
#include "../game/QueueMonitor.h"

#include "ServerReceiverThread.h"
#include "ServerSenderThread.h"

// Dueño de la conexión con un cliente: su ServerProtocol y los hilos de
// envío/recepción dedicados (ver ServerSenderThread/ServerReceiverThread).
class ClientHandler {
public:
    ClientHandler(uint16_t client_id, Socket&& socket,
                  Queue<std::shared_ptr<ServerCommand>>& command_queue, QueueMonitor& queue_monitor,
                  PersistenceMonitor& persistence_monitor, MapaDTO& initial_map);

    void start();
    void stop();
    void join();

    bool is_alive() const { return _alive; }
    uint16_t client_id() const { return _client_id; }

    ~ClientHandler() {}

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

private:
    uint16_t _client_id;
    ServerProtocol _protocol;
    std::atomic<bool> _alive;
    ServerSenderThread _sender;
    ServerReceiverThread _receiver;
    QueueMonitor& _queue_monitor;
    PersistenceMonitor& _persistence_monitor;
};
