#include "Acceptor.h"
#include <sys/socket.h>
#include <iostream>

Acceptor::Acceptor(const std::string& port,
                   Queue<std::shared_ptr<ServerCommand>>& command_queue,
                   QueueMonitor& queue_monitor)
    : socket(port.c_str()),
      command_queue(command_queue),
      queue_monitor(queue_monitor),
      running(false),
      next_id(1) {}

void Acceptor::run() {
    running = true;

    while (running) {
        try {
            Socket peer = socket.accept();

            this->next_id++;
            std::unique_ptr<ClientHandler> handler =
                std::make_unique<ClientHandler>(next_id, std::move(peer), command_queue, queue_monitor);

            handler->start();
            client_handlers.push_back(std::move(handler));

            reap_dead_clients();

        } catch (const std::exception& e) {
            if (running)
                std::cerr << "Acceptor error: " << e.what() << "\n";
        }
    }

}

void Acceptor::stop() {
    if (!this->running)
        return;
    this->running = false;

    this->socket.shutdown(SHUT_RDWR);
    this->socket.close();
    
    Thread::stop();

    for (std::unique_ptr<ClientHandler>& handler : this->client_handlers) {
        handler->stop();
        handler->join();
    }
    this->client_handlers.clear();
}

void Acceptor::reap_dead_clients() {
    this->client_handlers.remove_if([](const std::unique_ptr<ClientHandler>& handler) {
        if (!handler->is_alive()) {
            handler->stop();
            handler->join();
            return true;
        }
        return false;
    });
}