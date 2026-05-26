#include "ClientHandler.h"
#include <sys/socket.h>

ClientHandler::ClientHandler(uint16_t client_id,
                                                         Socket&& socket,
                                                         Queue<std::shared_ptr<ServerCommand>>& command_queue,
                                                         QueueMonitor& queue_monitor)
        : _client_id(client_id),
            _protocol(std::move(socket)),
            _alive(true),
            _receiver(_client_id, command_queue, _alive, _protocol),
            _sender(_protocol),
            _queue_monitor(queue_monitor) {}

void ClientHandler::start() {
    _receiver.start();
    _sender.start();

    _queue_monitor.add(this->client_id(), &this->_sender.get_queue());
}

void ClientHandler::stop() {
    if (!_alive)
        return;
    _alive = false;
    _protocol.shutdown(SHUT_RDWR);
    _queue_monitor.remove(this->client_id());
    _receiver.stop();
    _sender.stop();
}

void ClientHandler::join() {
    _receiver.join();
    _sender.join();
}