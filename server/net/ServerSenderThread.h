#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Serializer.h"
#include "../../common/protocol/dtos.h"

class ServerSenderThread : public Thread {
public:
    ServerSenderThread(Socket& socket, Queue<SnapshotDTO>& queue);

    void run() override;
    void stop() override;

private:
    Serializer          _serializer;
    Queue<SnapshotDTO>& _queue;
};