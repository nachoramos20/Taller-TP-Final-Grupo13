#pragma once

#include "../../common/queue.h"
#include "../../common/protocol/dtos.h"

#include <mutex>
#include <vector>
#include <cstdint>

class QueueMonitor {
public:
    void add(uint16_t client_id, Queue<SnapshotDTO>* queue);
    void remove(uint16_t client_id);
    void broadcast(const SnapshotDTO& snapshot);
    void send_to(uint16_t client_id, const SnapshotDTO& snapshot);

private:
    struct Entry {
        uint16_t            client_id;
        Queue<SnapshotDTO>* queue;
    };

    std::mutex         _mtx;
    std::vector<Entry> _queues;
};