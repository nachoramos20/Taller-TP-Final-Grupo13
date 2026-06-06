#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../net/ServerReceiverThread.h"
#include "QueueMonitor.h"
#include "Commands.h"
#include "World.h"
#include "Npc.h"

#include <chrono>
#include <thread>
#include <vector>

static constexpr int SERVER_TICK_RATE_HZ  = 30;
static constexpr int SERVER_TICK_MS       = 1000 / SERVER_TICK_RATE_HZ;
// Regen natural cada ~1 segundo (30 ticks)
static constexpr int REGEN_EVERY_N_TICKS  = SERVER_TICK_RATE_HZ;

class ServerGameLoop : public Thread {
public:
    ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                   QueueMonitor& queue_monitor, Queue<PlayerData>& save_queue);

    void run() override;
    void stop() override;

private:
    void process_commands();
    void update();
    void broadcast_snapshots();
    void cleanup_dead_npcs();

    static uint16_t hp_regen_per_interval(const PlayerData& p);
    static uint16_t mp_regen_per_interval(const PlayerData& p);

    Queue<std::shared_ptr<ServerCommand>>& command_queue;
    QueueMonitor&         queue_monitor;
    Queue<PlayerData>&    save_queue;
    World                 world;
    uint32_t              tick;
    int                   regen_ticks;
};
