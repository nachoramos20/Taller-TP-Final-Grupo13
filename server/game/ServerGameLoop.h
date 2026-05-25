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
    ServerGameLoop(Queue<ServerCommand>& command_queue,
                   QueueMonitor& queue_monitor)
        : _command_queue(command_queue),
          _queue_monitor(queue_monitor),
          _tick(0) {}

    void run() override {
        using clock = std::chrono::steady_clock;
        auto next_tick = clock::now();

        while (should_keep_running()) {
            ServerCommand cmd;
            while (_command_queue.try_pop(cmd)) {
                process_command(cmd);
            }

            broadcast_snapshots();

            _tick++;

            next_tick += std::chrono::milliseconds(SERVER_TICK_MS);
            std::this_thread::sleep_until(next_tick);
        }
    }

    void stop() override {
        Thread::stop();
    }

private:
    void process_command(const ServerCommand& cmd) {
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

    void broadcast_snapshots() {
        for (const auto& [client_id, player] : _world.players()) {
            SnapshotDTO snap = _world.build_snapshot(client_id, _tick);
            // Pushear directamente a la cola del cliente
            // TODO: usar QueueMonitor con snapshot personalizado por cliente
        }
        // Por ahora broadcast genérico
        SnapshotDTO snap;
        snap.tick = _tick;
        _queue_monitor.broadcast(snap);
    }

    Queue<ServerCommand>& _command_queue;
    QueueMonitor&         _queue_monitor;
    World                 _world;
    uint32_t              _tick;
};
