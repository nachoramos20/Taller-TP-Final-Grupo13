#pragma once

#include "../../common/socket.h"
#include "../../common/thread.h"
#include "ClientHandler.h"

#include <list>
#include <string>
#include <atomic>

class Acceptor : public Thread {
public:
    explicit Acceptor(const std::string& port);

    void run() override;
    void stop() override;

private:
    void reap_dead_clients();

    Socket                    _socket;
    std::list<ClientHandler*> _handlers;
    std::atomic<bool>         _running;
};