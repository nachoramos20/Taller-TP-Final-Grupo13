#include "ServerGameLoop.h"

#include "PlayerData.h"

#include <memory>
#include <unordered_map>
#include <vector>

ServerGameLoop::ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                               QueueMonitor& queue_monitor, Queue<PlayerData>& save_queue)
    : command_queue(command_queue),
      queue_monitor(queue_monitor),
      save_queue(save_queue),
      tick(0) {}

void ServerGameLoop::run() {
    using clock = std::chrono::steady_clock;
    std::chrono::time_point<clock> next_tick = clock::now();

    while (should_keep_running()) {
        game.revisar_colisiones();
        std::shared_ptr<ServerCommand> command;
        while (command_queue.try_pop(command)) {
            command->execute(game);
        }

        broadcast_snapshots();

        tick++;

        next_tick += std::chrono::milliseconds(SERVER_TICK_MS);
        std::this_thread::sleep_until(next_tick);
    }
}

void ServerGameLoop::stop() {
    Thread::stop();
}


void ServerGameLoop::broadcast_snapshots() {
    std::shared_ptr<std::vector<EntityDTO>> entities = game.get_entities();
    const std::unordered_map<uint16_t, PlayerData>& players = game.get_players();

    for (std::unordered_map<uint16_t, PlayerData>::const_iterator it = players.begin();
         it != players.end();
         ++it) {
        const uint16_t client_id = it->first;
        SnapshotDTO snap = game.build_snapshot(client_id, tick, entities);
        queue_monitor.send_to(client_id, snap);
    }
}