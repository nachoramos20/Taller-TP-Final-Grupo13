#include "ServerReceiverThread.h"
#include <iostream>

ServerReceiverThread::ServerReceiverThread(uint16_t client_id,
                                           Socket& socket,
                                           Queue<ServerCommand>& command_queue,
                                           std::atomic<bool>& client_alive)
    : _client_id(client_id),
      _deserializer(socket),
      _command_queue(command_queue),
      _client_alive(client_alive) {}

void ServerReceiverThread::run() {
    try {
        while (should_keep_running()) {
            MsgType type = _deserializer.recv_opcode();

            ServerCommand cmd = deserialize_command(type);
            cmd.client_id = _client_id;
            _command_queue.push(cmd);
        }
    } catch (const ClosedQueue&) {
        // Cola cerrada: shutdown normal
    } catch (const std::exception& e) {
        // Socket cerrado o error de red: el cliente se desconectó
        std::cerr << "Cliente " << _client_id << " desconectado: "
                  << e.what() << "\n";
    }

    // Marcar al cliente como muerto para que el Acceptor lo reapee.
    _client_alive = false;
}

void ServerReceiverThread::stop() {
    Thread::stop();
    // El socket se cierra desde ClientHandler::stop(),
    // lo que rompe el recv() y hace salir del loop
}

ServerCommand ServerReceiverThread::deserialize_command(MsgType type) {
    ServerCommand cmd;
    cmd.type = type;

    switch (type) {
        case MsgType::LOGIN:
            cmd.text = _deserializer.recv_str8();
            break;
        case MsgType::REGISTER:
            cmd.text = _deserializer.recv_str8();
            cmd.race = _deserializer.recv_uint8();
            cmd.cls  = _deserializer.recv_uint8();
            break;
        case MsgType::MOVE:
            cmd.pos_x = _deserializer.recv_uint16();
            cmd.pos_y = _deserializer.recv_uint16();
            break;
        case MsgType::ATTACK:
            cmd.target_id = _deserializer.recv_uint16();
            break;
        case MsgType::CHAT_COMMAND:
            cmd.text = _deserializer.recv_str8();
            break;
        case MsgType::EQUIP_ITEM:
        case MsgType::DROP_ITEM:
            cmd.slot = _deserializer.recv_uint8();
            break;
        case MsgType::PICK_ITEM:
        case MsgType::LOGOUT:
            // Sin payload
            break;
        default:
            break;
    }

    return cmd;
}
