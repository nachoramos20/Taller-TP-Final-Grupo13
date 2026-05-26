#include "ServerGameLoop.h"

ServerGameLoop::ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                               QueueMonitor& queue_monitor)
    : command_queue(command_queue),
      queue_monitor(queue_monitor),
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
    for (const auto& [client_id, player] : game.get_players()) {
        SnapshotDTO snap = game.build_snapshot(client_id, tick);
        queue_monitor.send_to(client_id, snap);
    }
}