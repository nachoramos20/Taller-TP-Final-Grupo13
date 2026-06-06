#include "ServerReceiverThread.h"
#include <iostream>
#include <memory>

ServerReceiverThread::ServerReceiverThread(uint16_t client_id,
                                           Queue<std::shared_ptr<ServerCommand>>& command_queue,
                                           QueueMonitor& queue_monitor,
                                           Queue<SnapshotDTO>& sender_queue,
                                           std::atomic<bool>& client_alive,
                                           ServerProtocol& server_protocol,
                                           PersistenceMonitor& persistence_monitor,
                                           MapaDTO& mapa)
    : client_id(client_id),
      server_protocol(server_protocol),
      command_queue(command_queue),
      queue_monitor(queue_monitor),
      sender_queue(sender_queue),
      client_alive(client_alive),
      persistence_monitor(persistence_monitor),
      mapa(mapa) {}

void ServerReceiverThread::run() {
    try {
        PlayerData player_data;
        bool ok = false;
        while (!ok && should_keep_running()) {
            MsgType type = server_protocol.receive_handshake();
            if (type == MsgType::LOGIN)         ok = handshake_login(player_data);
            else if (type == MsgType::REGISTER) ok = handshake_register(player_data);

            if (!ok) {
                server_protocol.send_login_error("Usuario invalido o ya existente");
                // No cortamos: dejamos que el cliente reintente
            }
        }

        server_protocol.send_login_ok(client_id);
        server_protocol.send_mapa(mapa);
        queue_monitor.add(client_id, &sender_queue);
        command_queue.push(std::make_shared<LoginCommand>(player_data));

        while (should_keep_running()) {
            auto cmd = server_protocol.receive_command(client_id);
            if (!cmd) { client_alive = false; break; }
            command_queue.push(cmd);
        }
        std::shared_ptr<LogoutCommand> logout = std::make_shared<LogoutCommand>(this->client_id);
        command_queue.push(logout);
    } catch (const ClosedQueue&) {
    } catch (const std::exception& e) {
        std::cerr << "Cliente " << client_id << " desconectado: " << e.what() << "\n";
    }
    client_alive = false;
}

void ServerReceiverThread::stop() {
    client_alive = false;
    Thread::stop();
}

bool ServerReceiverThread::handshake_login(PlayerData& player_data) {
    std::string username;
    server_protocol.handshake_login(username);
    return persistence_monitor.login(username, player_data, client_id);
}

bool ServerReceiverThread::handshake_register(PlayerData& player_data) {
    std::string username; uint8_t race, cls;
    server_protocol.handshake_register(username, race, cls);
    return persistence_monitor.register_user(username, race, cls, player_data, client_id);
}
