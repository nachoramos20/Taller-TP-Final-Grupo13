#include "ServerGameLoop.h"

#include "PlayerData.h"

#include <memory>
#include <unordered_map>
#include <vector>

ServerGameLoop::ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                               QueueMonitor& queue_monitor)
    : command_queue(command_queue), queue_monitor(queue_monitor), tick(0) {}

void ServerGameLoop::run() {
    using clock = std::chrono::steady_clock;
    auto next_tick = clock::now();

    while (should_keep_running()) {
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

void ServerGameLoop::stop() { Thread::stop(); }

void ServerGameLoop::broadcast_snapshots() {
    auto entities = game.get_entities();
    const auto& players = game.get_players();

    for (auto it = players.begin(); it != players.end(); ++it) {
        SnapshotDTO snap = game.build_snapshot(it->first, tick, entities);
        queue_monitor.send_to(it->first, snap);
    }
}
