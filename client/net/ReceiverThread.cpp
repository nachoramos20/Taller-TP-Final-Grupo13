#include "ReceiverThread.h"

ReceiverThread::ReceiverThread(Socket& socket, Queue<SnapshotDTO>& queue)
    : _deserializer(socket), _queue(queue), _my_entity_id(0) {}

void ReceiverThread::run() {
    try {
        while (should_keep_running()) {
            MsgType opcode = _deserializer.recv_opcode();

            switch (opcode) {
                case MsgType::LOGIN_OK:
                    _my_entity_id = _deserializer.recv_login_ok();
                    break;
                case MsgType::LOGIN_ERROR:
                    _deserializer.recv_login_error();
                    break;
                case MsgType::SNAPSHOT: {
                    SnapshotDTO snap = _deserializer.recv_snapshot();
                    _queue.push(std::move(snap));
                    break;
                }
                default:
                    break;
            }
        }
    } catch (const ClosedQueue&) {
    } catch (const std::exception&) {
    }
}

void ReceiverThread::stop() {
    Thread::stop();
}

uint16_t ReceiverThread::my_entity_id() const {
    return _my_entity_id;
}