#include "ServerGameLoop.h"
#include "PlayerData.h"

#include <memory>
#include <vector>

ServerGameLoop::ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                               QueueMonitor& queue_monitor, Queue<PlayerData>& save_queue)
    : command_queue(command_queue),
      queue_monitor(queue_monitor),
      save_queue(save_queue),
      world(100, 100), tick(0) {}

void ServerGameLoop::run() {
    using clock = std::chrono::steady_clock;
    auto next_tick = clock::now();

    while (should_keep_running()) {
        process_commands();
        update();
        broadcast_snapshots();

        tick++;
        if (tick % 60 == 0) {
            save_players();
        }
        next_tick += std::chrono::milliseconds(SERVER_TICK_MS);
        std::this_thread::sleep_until(next_tick);
    }
}

void ServerGameLoop::stop() { Thread::stop(); }

void ServerGameLoop::process_commands() {
    std::shared_ptr<ServerCommand> command;
    while (command_queue.try_pop(command)) {
        command->execute(world);
    }
}

void ServerGameLoop::update() {
    // tick de cooldowns de ataque
    for (auto& [id, player] : world.get_players_mutable()) {
        if (player.attack_cooldown > 0)
            player.attack_cooldown--;
    }

    // tick de meditación — regenerar MP por tick
    for (auto& [id, player] : world.get_players_mutable()) {
        if (player.meditating && !player.is_ghost) {
            player.mp = std::min(player.max_mp,
                static_cast<uint16_t>(player.mp + (player.intelligence / 10 + 1)));
        }
    }
}

void ServerGameLoop::broadcast_snapshots() {
    auto entities = world.get_entities();
    const auto& players = world.get_players();

    for (const auto& [client_id, player] : players) {
        SnapshotDTO snap = world.build_snapshot(client_id, tick, entities);
        queue_monitor.send_to(client_id, snap);
    }
}

void ServerGameLoop::save_players() {
    const auto& players = world.get_players();
    for (const auto& [client_id, player] : players) {
        save_queue.push(player);
    }
}