#pragma once

#include <functional>
#include <unordered_map>

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "ClientProtocol.h"
#include "Command.h"

// Hilo dedicado a drenar la cola de Command's del jugador y mandarlos por
// el socket, para que la UI nunca bloquee escribiendo a red.
class SenderThread : public Thread {
public:
    SenderThread(ClientProtocol& protocol, Queue<Command>& queue);

    void run() override;
    void stop() override;

private:
    void send_command(const Command& cmd);

    // Cada entrada llama al send_* de ClientProtocol que
    // corresponde a ese tipo de comando.
    using SendAction = std::function<void(SenderThread&, const Command&)>;
    static const std::unordered_map<MsgType, SendAction>& dispatch_table();

    ClientProtocol& _protocol;
    Queue<Command>& _queue;
};
