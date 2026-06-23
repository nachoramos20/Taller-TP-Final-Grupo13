#include "ClientHandler.h"
#include <sys/socket.h>

ClientHandler::ClientHandler(uint16_t client_id,
                             Socket&& socket,
                             Queue<std::shared_ptr<ServerCommand>>& command_queue,
                             QueueMonitor& queue_monitor,
                             PersistenceMonitor& persistence_monitor,
                             MapaDTO& initial_map)
    : _client_id(client_id),
      _protocol(std::move(socket)),
      _alive(true),
      _sender(_protocol),
      _receiver(_client_id, command_queue, queue_monitor, _sender.get_queue(),
                _alive, _protocol, persistence_monitor, initial_map),
      _queue_monitor(queue_monitor),
      _persistence_monitor(persistence_monitor) {}

void ClientHandler::start() {
    _receiver.start();
    _sender.start();
}

void ClientHandler::stop() {
    _alive = false;
    try {
        _protocol.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
    }
    _queue_monitor.remove(this->client_id());
    _receiver.stop();
    _sender.stop();
}

void ClientHandler::join() {
    _receiver.join();
    _sender.join();
}
