#pragma once

#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../game/QueueMonitor.h"
#include "../game/PersistenceMonitor.h"
#include "ClientHandler.h"

#include <list>
#include <string>
#include <atomic>
#include <cstdint>

class Acceptor : public Thread {
public:
    Acceptor(const std::string& port,
             Queue<std::shared_ptr<ServerCommand>>& command_queue,
             QueueMonitor& queue_monitor,
             PersistenceMonitor& persistence_monitor);

    void run() override;
    void stop() override;

private:
    void reap_dead_clients();

    Socket                    socket;
    Queue<std::shared_ptr<ServerCommand>>&     command_queue;
    QueueMonitor&             queue_monitor;
    PersistenceMonitor&       persistence_monitor;
    std::list<std::unique_ptr<ClientHandler>> client_handlers;
    std::atomic<bool>         running;
    uint16_t                  next_id;
};