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

    MsgType handshake_type = this->server_protocol.receive_handshake();
    PlayerData player_data;
    bool auth = false;
    

    switch (handshake_type) {
        case MsgType::LOGIN:
            auth = handshake_login(player_data);
            break;
        case MsgType::REGISTER:
            auth = handshake_register(player_data);
            break;
    }

    if (!auth) {
        server_protocol.send_login_error("Login failed: invalid username or user already exists");
        throw std::runtime_error("Handshake failed: authentication error");
    }

    server_protocol.send_login_ok(this->client_id);
    server_protocol.send_mapa(mapa); 

    queue_monitor.add(this->client_id, &this->sender_queue);
    this->command_queue.push(std::make_shared<LoginCommand>(player_data));
}

bool ServerReceiverThread::handshake_login(PlayerData& player_data) {
    std::string username;
    server_protocol.handshake_login(username);
    return persistence_monitor.login(username, player_data, this->client_id);
}

bool ServerReceiverThread::handshake_register(PlayerData& player_data) {
    std::string username;
    uint8_t race;
    uint8_t cls;
    server_protocol.handshake_register(username, race, cls);
    return persistence_monitor.register_user(username, race, cls, player_data, this->client_id);
}