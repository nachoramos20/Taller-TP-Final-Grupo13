#pragma once

#include "../../common/queue.h"
#include "../../common/protocol/dtos.h"

#include <mutex>
#include <vector>
#include <algorithm>
#include <cstdint>

class QueueMonitor {
public:
    void add(uint16_t client_id, Queue<SnapshotDTO>* queue) {
        std::lock_guard<std::mutex> lock(_mtx);
        _queues.push_back({client_id, queue});
    }

    void remove(uint16_t client_id) {
        std::lock_guard<std::mutex> lock(_mtx);
        _queues.erase(
            std::remove_if(_queues.begin(), _queues.end(),
                [client_id](const Entry& e) {
                    return e.client_id == client_id;
                }),
            _queues.end()
        );
    }

    void broadcast(const SnapshotDTO& snapshot) {
        std::lock_guard<std::mutex> lock(_mtx);
        for (auto& entry : _queues) {
            try {
                entry.queue->push(snapshot);
            } catch (const ClosedQueue&) {}
        }
    }

private:
    struct Entry {
        uint16_t            client_id;
        Queue<SnapshotDTO>* queue;
    };

    std::mutex         _mtx;
    std::vector<Entry> _queues;
};