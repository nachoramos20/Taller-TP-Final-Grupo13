#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/dtos.h"
#include "ServerProtocol.h"

// Hilo dedicado a drenar la cola de snapshots de un cliente y mandarlos
// por su socket, para que ServerGameLoop nunca bloquee escribiendo a red.
class ServerSenderThread : public Thread {
public:
    explicit ServerSenderThread(ServerProtocol& protocol);

    Queue<SnapshotDTO>& get_queue();

    void run() override;
    void stop() override;

private:
    ServerProtocol&       server_protocol;
    Queue<SnapshotDTO>   queue;
};