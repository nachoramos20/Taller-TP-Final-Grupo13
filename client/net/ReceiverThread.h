#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Deserializer.h"
#include "../../common/protocol/dtos.h"
#include "../../common/MapaDTO.h"

#include <atomic>

class ReceiverThread : public Thread {
public:
    ReceiverThread(Socket& socket,
                   Queue<SnapshotDTO>& snapshot_queue,
                   Queue<MapaDTO>& map_queue,
                   std::atomic<bool>& connected);

    void run() override;
    void stop() override;

    uint16_t my_entity_id() const;

private:
    Deserializer        _deserializer;
    Queue<SnapshotDTO>& _snapshot_queue;
    Queue<MapaDTO>&     _map_queue;
    std::atomic<bool>&  _connected;
    uint16_t            _my_entity_id;
};