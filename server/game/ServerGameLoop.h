#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../net/ServerReceiverThread.h"
#include "QueueMonitor.h"
#include "World.h"

#include <chrono>
#include <thread>

static constexpr int SERVER_TICK_RATE_HZ = 30;
static constexpr int SERVER_TICK_MS = 1000 / SERVER_TICK_RATE_HZ;

class ServerGameLoop : public Thread {
public:
    ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                   QueueMonitor& queue_monitor);

    void run() override;
    void stop() override;

private:
    void process_command(const ServerCommand& cmd);
    void broadcast_snapshots();

    Queue<std::shared_ptr<ServerCommand>>& _command_queue;
    QueueMonitor&         _queue_monitor;
    World                 _world;
    uint32_t              _tick;
};