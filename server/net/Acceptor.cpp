#include "Acceptor.h"
#include <sys/socket.h>
#include <iostream>

Acceptor::Acceptor(const std::string& port)
    : _socket(port.c_str()), _running(false) {}

void Acceptor::run() {
    _running = true;

    while (_running) {
        try {
            Socket peer = _socket.accept();

            ClientHandler* handler = new ClientHandler(std::move(peer));
            handler->start();
            _handlers.push_back(handler);

            reap_dead_clients();

        } catch (const std::exception& e) {
            if (_running) {
                std::cerr << "Acceptor error: " << e.what() << "\n";
            }
        }
    }

    for (ClientHandler* h : _handlers) {
        h->stop();
        h->join();
        delete h;
    }
    _handlers.clear();
}

void Acceptor::stop() {
    _running = false;
    Thread::stop();
    _socket.shutdown(SHUT_RDWR);
}

void Acceptor::reap_dead_clients() {
    auto it = _handlers.begin();
    while (it != _handlers.end()) {
        ClientHandler* h = *it;
        if (!h->is_alive()) {
            h->stop();
            h->join();
            delete h;
            it = _handlers.erase(it);
        } else {
            ++it;
        }
    }
}