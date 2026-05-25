#include "ServerGameLoop.h"

ServerGameLoop::ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                               QueueMonitor& queue_monitor)
    : _command_queue(command_queue),
      _queue_monitor(queue_monitor),
      _tick(0) {}

void ServerGameLoop::run() {
    using clock = std::chrono::steady_clock;
    auto next_tick = clock::now();

    while (should_keep_running()) {
        std::shared_ptr<ServerCommand> cmd;
        while (_command_queue.try_pop(cmd)) {
            process_command(*cmd);
        }

        broadcast_snapshots();

        _tick++;

        next_tick += std::chrono::milliseconds(SERVER_TICK_MS);
        std::this_thread::sleep_until(next_tick);
    }
}

void ServerGameLoop::stop() {
    Thread::stop();
}

void ServerGameLoop::process_command(const ServerCommand& cmd) {
    switch (cmd.type) {
        case MsgType::LOGIN:
        case MsgType::REGISTER:
            if (!_world.has_player(cmd.client_id))
                _world.add_player(cmd.client_id);
            break;
        case MsgType::MOVE:
            _world.move_player(cmd.client_id, cmd.pos_x, cmd.pos_y);
            break;
        case MsgType::LOGOUT:
            _world.remove_player(cmd.client_id);
            break;
        default:
            break;
    }
}

void ServerGameLoop::broadcast_snapshots() {
    for (const auto& [client_id, player] : _world.players()) {
        SnapshotDTO snap = _world.build_snapshot(client_id, _tick);
        _queue_monitor.send_to(client_id, snap);
    }
}