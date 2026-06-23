#include "ServerReceiverThread.h"
#include <iostream>
#include <memory>
#include <string>

ServerReceiverThread::ServerReceiverThread(uint16_t client_id,
                                           Queue<std::shared_ptr<ServerCommand>>& command_queue,
                                           QueueMonitor& queue_monitor,
                                           Queue<SnapshotDTO>& sender_queue,
                                           std::atomic<bool>& client_alive,
                                           ServerProtocol& server_protocol,
                                           PersistenceMonitor& persistence_monitor,
                                           MapaDTO& initial_map)
    : _client_id(client_id),
      _server_protocol(server_protocol),
      _command_queue(command_queue),
      _queue_monitor(queue_monitor),
      _sender_queue(sender_queue),
      _client_alive(client_alive),
      _persistence_monitor(persistence_monitor),
      _initial_map(initial_map) {}

void ServerReceiverThread::run() {
    try {
        PlayerData player_data;
        bool ok = false;
        while (!ok && should_keep_running()) {
            MsgType type = _server_protocol.receive_handshake();
            if (type == MsgType::LOGIN)          ok = handshake_login(player_data);
            else if (type == MsgType::REGISTER) ok = handshake_register(player_data);

            if (!ok) {
                _server_protocol.send_login_error("Usuario invalido o ya existente");
            }
        }

        _server_protocol.send_login_ok(_client_id);
        _server_protocol.send_mapa(_initial_map);
        _queue_monitor.add(_client_id, &_sender_queue);
        _command_queue.push(std::make_shared<LoginCommand>(player_data));

        while (should_keep_running()) {
            std::shared_ptr<ServerCommand> command = _server_protocol.receive_command(_client_id);
            if (!command) { _client_alive = false; break; }
            _command_queue.push(command);
        }
        std::shared_ptr<LogoutCommand> logout = std::make_shared<LogoutCommand>(_client_id);
        _command_queue.push(logout);
    } catch (const ClosedQueue&) {
    } catch (const std::exception& e) {
        // "Handshake: peer closed" pasa siempre que alguien se conecta y
        // cierra sin mandar nada (el chequeo de conectividad del cliente al
        // arrancar, o una conexión de reintento de login abandonada): no es
        // un error real, así que no lo logueamos como tal.
        std::string msg = e.what();
        if (msg != "Handshake: peer closed")
            std::cerr << "Cliente " << _client_id << " desconectado: " << msg << "\n";
    }
    _client_alive = false;
}

void ServerReceiverThread::stop() {
    _client_alive = false;
    Thread::stop();
}

bool ServerReceiverThread::handshake_login(PlayerData& player_data) {
    std::string username;
    _server_protocol.handshake_login(username);
    return _persistence_monitor.login(username, player_data, _client_id);
}

bool ServerReceiverThread::handshake_register(PlayerData& player_data) {
    std::string username;
    uint8_t race_id;
    uint8_t class_id;
    _server_protocol.handshake_register(username, race_id, class_id);
    return _persistence_monitor.register_user(username, race_id, class_id, player_data, _client_id);
}
