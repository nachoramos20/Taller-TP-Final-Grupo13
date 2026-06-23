#include "Acceptor.h"
#include <sys/socket.h>
#include <iostream>

Acceptor::Acceptor(const std::string& port,
                   Queue<std::shared_ptr<ServerCommand>>& command_queue,
                   QueueMonitor& queue_monitor,
                   PersistenceMonitor& persistence_monitor,
                   MapaDTO& initial_map)
    : _listener_socket(port.c_str()),
      _command_queue(command_queue),
      _queue_monitor(queue_monitor),
      _persistence_monitor(persistence_monitor),
      _running(false),
      _next_client_id(1),
      _initial_map(initial_map) {}

void Acceptor::run() {
    _running = true;

    while (_running) {
        try {
            Socket client_socket = _listener_socket.accept();

            ++_next_client_id;
            std::unique_ptr<ClientHandler> client_handler =
                std::make_unique<ClientHandler>(_next_client_id, std::move(client_socket),
                                                _command_queue, _queue_monitor,
                                                _persistence_monitor, _initial_map);

            client_handler->start();
            _client_handlers.push_back(std::move(client_handler));

            reap_dead_clients();

        } catch (const std::exception& e) {
            if (_running)
                std::cerr << "Acceptor error: " << e.what() << "\n";
        }
    }

}

void Acceptor::stop() {
    if (!_running)
        return;
    _running = false;

    _listener_socket.shutdown(SHUT_RDWR);
    _listener_socket.close();
    
    Thread::stop();

    for (std::unique_ptr<ClientHandler>& handler : _client_handlers) {
        handler->stop();
        handler->join();
    }
    _client_handlers.clear();
}

void Acceptor::reap_dead_clients() {
    _client_handlers.remove_if([](const std::unique_ptr<ClientHandler>& handler) {
        if (!handler->is_alive()) {
            handler->stop();
            handler->join();
            return true;
        }
        return false;
    });
}
