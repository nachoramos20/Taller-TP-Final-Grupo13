#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Deserializer.h"
#include "../../common/protocol/dtos.h"
#include "../../common/MapaDTO.h"

#include <atomic>
#include <string>

enum class HandshakeResult { PENDING, OK, ERROR };

class ReceiverThread : public Thread {
public:
    ReceiverThread(Socket& socket,
                   Queue<SnapshotDTO>& snapshot_queue,
                   Queue<MapaDTO>& map_queue,
                   std::atomic<bool>& connected);

    void run() override;
    void stop() override;

    uint16_t my_entity_id() const;

    HandshakeResult wait_handshake(std::string& error_msg);

private:
    void game_loop_receive();

    Deserializer        _deserializer;
    Queue<SnapshotDTO>& _snapshot_queue;
    Queue<MapaDTO>&     _map_queue;
    std::atomic<bool>&  _connected;
    uint16_t            _my_entity_id;

    Queue<std::string>  _login_ok_queue;    // contiene entity_id como string "OK:<id>"
    Queue<std::string>  _login_error_queue; // contiene el mensaje de error
};
