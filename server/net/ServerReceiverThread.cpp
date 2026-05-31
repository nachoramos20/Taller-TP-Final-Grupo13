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
        handshake_client();
        while (should_keep_running()) {
            std::shared_ptr<ServerCommand> cmd = server_protocol.receive_command(this->client_id);
            if (cmd) {
                this->command_queue.push(cmd);
            } else {
                this->client_alive = false;
                break;
            }
        }
    } catch (const ClosedQueue&) {
    } catch (const std::exception& e) {
        std::cerr << "Cliente " << client_id
                  << " desconectado: " << e.what() << "\n";
    }

    client_alive = false;
}

void ServerReceiverThread::stop() {
    client_alive = false;
    Thread::stop();
}

void ServerReceiverThread::handshake_client() {
    std::string username;
    uint8_t race = 0;
    uint8_t cls = 0;
    PlayerData player_data;

    MsgType handshake_type = this->server_protocol.handshake(username, race, cls);
    bool authenticated = false;

    if (handshake_type == MsgType::LOGIN) {
        authenticated = persistence_monitor.login(username, player_data, this->client_id);
        if (!authenticated) {
            server_protocol.send_login_error("Usuario inexistente");
            client_alive = false;
            return;
        }
    } else if (handshake_type == MsgType::REGISTER) {
        authenticated = persistence_monitor.register_user(username, race, cls, player_data, this->client_id);
        if (!authenticated) {
            server_protocol.send_login_error("Nombre de usuario ya ocupado");
            client_alive = false;
            return;
        }
    }

    server_protocol.send_login_ok(this->client_id);
    
    queue_monitor.add(this->client_id, &this->sender_queue);
    this->command_queue.push(std::make_shared<LoginCommand>(player_data));
}