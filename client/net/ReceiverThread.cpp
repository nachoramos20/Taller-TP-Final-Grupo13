#include "ReceiverThread.h"

ReceiverThread::ReceiverThread(Socket& socket,
                               Queue<SnapshotDTO>& snapshot_queue,
                               Queue<MapaDTO>& map_queue,
                               std::atomic<bool>& connected)
    : _deserializer(socket),
      _snapshot_queue(snapshot_queue),
      _map_queue(map_queue),
      _connected(connected),
      _my_entity_id(0) {}

void ReceiverThread::run() {
    try {
        while (should_keep_running()) {
            MsgType opcode = _deserializer.recv_opcode();
            switch (opcode) {
                case MsgType::LOGIN_OK:
                    _my_entity_id = _deserializer.recv_login_ok();
                    break;
                case MsgType::LOGIN_ERROR: {
                    std::string err = _deserializer.recv_login_error();
                    break;
}
                case MsgType::MAPA: {
                    MapaDTO map = _deserializer.recv_map();
                    _map_queue.push(std::move(map));
                    break;
                }
                case MsgType::SNAPSHOT: {
                    SnapshotDTO snap = _deserializer.recv_snapshot();
                    _snapshot_queue.push(std::move(snap));
                    break;
                }
                default:
                    break;
            }
        }
    } catch (const ClosedQueue&) {
    } catch (const std::exception&) {
    }
    _connected = false;
}

void ReceiverThread::stop() { Thread::stop(); }

uint16_t ReceiverThread::my_entity_id() const { return _my_entity_id; }