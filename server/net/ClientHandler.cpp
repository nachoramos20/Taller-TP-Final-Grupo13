#include "ClientHandler.h"
#include <sys/socket.h>

ClientHandler::ClientHandler(uint16_t client_id,
                             Socket&& socket,
                             Queue<std::shared_ptr<ServerCommand>>& command_queue)
    : _client_id(client_id),
      _socket(std::move(socket)),
      _alive(true),
      _receiver(_client_id, _socket, command_queue, _alive),
      _sender(_socket, _snapshot_queue) {}

void ClientHandler::start() {
    _receiver.start();
    _sender.start();
}

void ClientHandler::stop() {
    _alive = false;
    _socket.shutdown(SHUT_RDWR);
    _snapshot_queue.close();
    _receiver.stop();
    _sender.stop();
}

void ClientHandler::join() {
    _receiver.join();
    _sender.join();
}