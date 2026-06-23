#pragma once

#include "../../common/queue.h"
#include "../../common/protocol/dtos.h"

#include <mutex>
#include <vector>
#include <cstdint>

// Registro thread-safe de la cola de snapshots saliente de cada cliente
// conectado, para que ServerGameLoop pueda mandarle un snapshot a todos
// (broadcast) o a uno solo (send_to) sin conocer el resto de la conexión.
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