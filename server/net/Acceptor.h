#pragma once

#include "../../common/socket.h"
#include "../../common/thread.h"
#include "ClientHandler.h"
#include "ServerReceiverThread.h"

#include <list>
#include <string>
#include <atomic>
#include <cstdint>

class Acceptor : public Thread {
public:
    Acceptor(const std::string& port, Queue<ServerCommand>& command_queue);

    void run() override;
    void stop() override;

private:
    void reap_dead_clients();

    Socket                    _socket;
    Queue<ServerCommand>&     _command_queue;
    std::list<ClientHandler*> _handlers;
    std::atomic<bool>         _running;
    uint16_t                  _next_id;
};