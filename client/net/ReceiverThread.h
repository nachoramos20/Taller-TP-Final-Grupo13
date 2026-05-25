#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Deserializer.h"
#include "../../common/protocol/dtos.h"

class ReceiverThread : public Thread {
public:
    ReceiverThread(Socket& socket, Queue<SnapshotDTO>& queue);

    void run() override;
    void stop() override;

    uint16_t my_entity_id() const;

private:
    Deserializer        _deserializer;
    Queue<SnapshotDTO>& _queue;
    uint16_t            _my_entity_id;
};