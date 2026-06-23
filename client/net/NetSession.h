#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "../../common/protocol/MapaDTO.h"
#include "../../common/protocol/dtos.h"
#include "../../common/queue.h"
#include "ClientProtocol.h"
#include "Command.h"
#include "ReceiverThread.h"
#include "SenderThread.h"

// Agrupa el socket de conexión al server, sus threads de envío/recepción y
// las colas que los conectan con el GameLoop. `connect()` abre el socket y
// arranca los threads; `authenticate()` hace el handshake de login/registro
// sobre esos threads ya corriendo. Una instancia es de un solo uso: si
// falla el handshake o se cierra la sesión, hay que crear una nueva.
class NetSession {
public:
    NetSession() = default;
    ~NetSession();

    bool connect(const std::string& host, const std::string& port, std::string& error_message);
    bool authenticate(const std::string& username, bool should_register,
                      uint8_t race_id, uint8_t class_id, std::string& error_message);
    void shutdown();

    Queue<Command>& commands() { return _command_queue; }
    Queue<SnapshotDTO>& snapshots() { return _snapshot_queue; }
    Queue<MapaDTO>& maps() { return _map_queue; }
    std::atomic<bool>& connection_state() { return _connected; }

    NetSession(const NetSession&) = delete;
    NetSession& operator=(const NetSession&) = delete;

private:
    void start_threads();

    std::unique_ptr<ClientProtocol> _protocol;
    Queue<Command> _command_queue;
    Queue<SnapshotDTO> _snapshot_queue;
    Queue<MapaDTO> _map_queue;
    std::atomic<bool> _connected{false};
    std::unique_ptr<SenderThread> _sender_thread;
    std::unique_ptr<ReceiverThread> _receiver_thread;
};
