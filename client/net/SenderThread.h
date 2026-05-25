#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Serializer.h"
#include "Command.h"

class SenderThread : public Thread {
public:
    SenderThread(Socket& socket, Queue<Command>& queue);

    void run() override;
    void stop() override;

private:
    void send_command(const Command& cmd);

    Serializer      _serializer;
    Queue<Command>& _queue;
};