#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Deserializer.h"
#include "../../common/protocol/dtos.h"

class ReceiverThread : public Thread {
public:
    ReceiverThread(Socket& socket, Queue<SnapshotDTO>& queue)
        : _deserializer(socket), _queue(queue) {}

    void run() override {
        try {
            while (should_keep_running()) {
                MsgType opcode = _deserializer.recv_opcode();

                switch (opcode) {
                    case MsgType::LOGIN_OK:
                        _my_entity_id = _deserializer.recv_login_ok();
                        break;
                    case MsgType::LOGIN_ERROR:
                        _deserializer.recv_login_error();
                        // TODO: notificar al GameLoop
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

    void stop() override {
        Thread::stop();
    }

    uint16_t my_entity_id() const { return _my_entity_id; }

private:
    Deserializer        _deserializer;
    Queue<SnapshotDTO>& _queue;
    uint16_t            _my_entity_id = 0;
};