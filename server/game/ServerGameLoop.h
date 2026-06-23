#pragma once

#include <chrono>
#include <thread>
#include <vector>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../net/ServerReceiverThread.h"
#include "commands/Commands.h"
#include "world/World.h"

#include "Npc.h"
#include "QueueMonitor.h"

// El tick del servidor: por cada vuelta procesa los comandos encolados,
// actualiza el mundo (NPCs, regeneración, items) y difunde un snapshot a
// cada cliente conectado. Dueño de la única instancia de World.
class ServerGameLoop: public Thread {
public:
    ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                   QueueMonitor& queue_monitor, Queue<PlayerData>& save_queue,
                   std::vector<uint8_t> collision_map);

    void run() override;
    void stop() override;

private:
    void process_commands();
    void update();
    void broadcast_snapshots();

    static uint16_t hp_regen_per_interval(const PlayerData& p);
    static uint16_t mp_regen_per_interval(const PlayerData& p);
    void save_players();

    Queue<std::shared_ptr<ServerCommand>>& command_queue;
    QueueMonitor& queue_monitor;
    Queue<PlayerData>& save_queue;
    World world;
    uint32_t tick;
    int regen_ticks;

    // Cadencia del loop, leída de GameConfig (formulas.tick_rate_hz) al construir.
    int tick_ms;
    int regen_every_n_ticks;
};