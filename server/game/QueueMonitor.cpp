#include "QueueMonitor.h"
#include <algorithm>

void QueueMonitor::add(uint16_t client_id, Queue<SnapshotDTO>* queue) {
    std::lock_guard<std::mutex> lock(_mtx);
    _queues.push_back({client_id, queue});
}

void QueueMonitor::remove(uint16_t client_id) {
    std::lock_guard<std::mutex> lock(_mtx);
    _queues.erase(
        std::remove_if(_queues.begin(), _queues.end(),
            [client_id](const Entry& e) {
                return e.client_id == client_id;
            }),
        _queues.end()
    );
}

void QueueMonitor::broadcast(const SnapshotDTO& snapshot) {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& entry : _queues) {
        try {
            entry.queue->push(snapshot);
        } catch (const ClosedQueue&) {}
    }
}

void QueueMonitor::send_to(uint16_t client_id, const SnapshotDTO& snapshot) {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& entry : _queues) {
        if (entry.client_id == client_id) {
            try {
                entry.queue->push(snapshot);
            } catch (const ClosedQueue&) {}
            return;
        }
    }
}