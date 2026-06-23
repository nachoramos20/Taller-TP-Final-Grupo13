#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/protocol.h"
#include "../../common/protocol/dtos.h"
#include "../game/commands/Commands.h"
#include "../game/QueueMonitor.h"
#include "../game/PersistenceMonitor.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <memory>
#include "ServerProtocol.h"

// Hilo dedicado a leer del socket de un cliente: hace el handshake
// (login/registro) y después encola cada comando recibido para que
// ServerGameLoop lo procese.
class ServerReceiverThread : public Thread {
public:
    ServerReceiverThread(uint16_t client_id,
                         Queue<std::shared_ptr<ServerCommand>>& command_queue,
                         QueueMonitor& queue_monitor,
                         Queue<SnapshotDTO>& sender_queue,
                         std::atomic<bool>& client_alive,
                         ServerProtocol& server_protocol,
                         PersistenceMonitor& persistence_monitor,
                         MapaDTO& initial_map);

    void run() override;
    void stop() override;

private:
    uint16_t _client_id;
    ServerProtocol& _server_protocol;
    Queue<std::shared_ptr<ServerCommand>>& _command_queue;
    QueueMonitor& _queue_monitor;
    Queue<SnapshotDTO>& _sender_queue;
    std::atomic<bool>& _client_alive;
    PersistenceMonitor& _persistence_monitor;
    MapaDTO& _initial_map;

    void handshake_client();
    bool handshake_login(PlayerData& player_data);
    bool handshake_register(PlayerData& player_data);
};
