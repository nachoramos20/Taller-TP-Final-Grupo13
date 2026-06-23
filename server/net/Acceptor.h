#pragma once

#include <atomic>
#include <cstdint>
#include <list>
#include <string>

#include "../../common/protocol/MapaDTO.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../game/PersistenceMonitor.h"
#include "../game/QueueMonitor.h"

#include "ClientHandler.h"

// Escucha conexiones entrantes y crea un ClientHandler por cada una, en
// su propio hilo. También da de baja a los handlers cuyo hilo ya terminó.
class Acceptor: public Thread {
public:
    Acceptor(const std::string& port, Queue<std::shared_ptr<ServerCommand>>& command_queue,
             QueueMonitor& queue_monitor, PersistenceMonitor& persistence_monitor,
             MapaDTO& initial_map);

    void run() override;
    void stop() override;

private:
    void reap_dead_clients();

    Socket _listener_socket;
    Queue<std::shared_ptr<ServerCommand>>& _command_queue;
    QueueMonitor& _queue_monitor;
    PersistenceMonitor& _persistence_monitor;
    std::list<std::unique_ptr<ClientHandler>> _client_handlers;
    std::atomic<bool> _running;
    uint16_t _next_client_id;
    MapaDTO& _initial_map;
};
