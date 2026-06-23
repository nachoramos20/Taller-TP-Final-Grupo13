#pragma once

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "ClientProtocol.h"
#include "Command.h"

class SenderThread : public Thread {
public:
    SenderThread(ClientProtocol& protocol, Queue<Command>& queue);

    void run() override;
    void stop() override;

private:
    void send_command(const Command& cmd);

    ClientProtocol& _protocol;
    Queue<Command>& _queue;
};
