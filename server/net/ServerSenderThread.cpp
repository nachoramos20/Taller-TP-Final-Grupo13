#include "ServerSenderThread.h"

ServerSenderThread::ServerSenderThread(Socket& socket, Queue<SnapshotDTO>& queue)
    : _serializer(socket), _queue(queue) {}

void ServerSenderThread::run() {
    try {
        while (true) {
            SnapshotDTO snap = _queue.pop();
            _serializer.send_snapshot(snap);
        }
    } catch (const ClosedQueue&) {
        // shutdown normal
    } catch (const std::exception&) {
        // socket cerrado
    }
}

void ServerSenderThread::stop() {
    Thread::stop();
}