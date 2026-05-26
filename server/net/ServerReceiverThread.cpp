#include "ServerReceiverThread.h"
#include <iostream>

ServerReceiverThread::ServerReceiverThread(uint16_t client_id,
                                           Queue<std::shared_ptr<ServerCommand>>& command_queue,
                                           std::atomic<bool>& client_alive, ServerProtocol& server_protocol)
        : client_id(client_id),
            server_protocol(server_protocol),
            command_queue(command_queue),
            client_alive(client_alive) {}

void ServerReceiverThread::run() {
    try {
        while (should_keep_running()) {
            std::shared_ptr<ServerCommand> cmd = server_protocol.receive_command(this->client_id);
            if (cmd) {
                this->command_queue.push(cmd);
            } else {
                this->client_alive = false;
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
    if (!this->client_alive)
        return;
    Thread::stop();
}