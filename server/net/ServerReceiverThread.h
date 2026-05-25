#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Deserializer.h"
#include "../../common/protocol/protocol.h"

#include <atomic>
#include <cstdint>

// Comando del servidor: acción de un jugador con su client_id
struct ServerCommand {
    uint16_t client_id;
    MsgType  type;

    // Payload según el tipo
    uint16_t    pos_x     = 0;
    uint16_t    pos_y     = 0;
    uint16_t    target_id = 0;
    uint8_t     slot      = 0;
    uint8_t     race      = 0;
    uint8_t     cls       = 0;
    std::string text;
};

// ReceiverThread del servidor: lee comandos del socket del cliente
// y los pushea a la cola global del GameLoop con el client_id inyectado.
class ServerReceiverThread : public Thread {
public:
    ServerReceiverThread(uint16_t client_id,
                         Socket& socket,
                         Queue<ServerCommand>& command_queue,
                         std::atomic<bool>& client_alive);

    void run() override;
    void stop() override;

private:
    ServerCommand deserialize_command(MsgType type);

    uint16_t              _client_id;
    Deserializer          _deserializer;
    Queue<ServerCommand>& _command_queue;
    std::atomic<bool>&    _client_alive;
};