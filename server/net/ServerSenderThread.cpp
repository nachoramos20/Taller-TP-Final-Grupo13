#include "ServerSenderThread.h"

ServerSenderThread::ServerSenderThread(ServerProtocol& protocol)
    : server_protocol(protocol), queue() {}

Queue<SnapshotDTO>& ServerSenderThread::get_queue() {
    return queue;
}

void ServerSenderThread::run() {
    try {
        while (true) {
            SnapshotDTO snap = queue.pop();
            server_protocol.send_snapshot(snap);
        }
    } catch (const ClosedQueue&) {
        // shutdown normal
    } catch (const std::exception&) {
        // socket cerrado
    }
}

void ServerSenderThread::stop() {
    this->queue.close();
    Thread::stop();
}