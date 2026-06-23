#ifndef PERSISTENCE_THREAD_H
#define PERSISTENCE_THREAD_H

#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/dtos.h"
#include "entities/PlayerData.h"
#include "PersistenceMonitor.h"

// Drena save_queue y persiste cada PlayerData en su propio hilo, para que
// guardar a disco no bloquee a los hilos que atienden clientes.
class PersistenceThread : public Thread {
public:
    PersistenceThread(Queue<PlayerData>& save_queue, PersistenceMonitor& persistence_monitor);

    void run() override;
    void stop() override;

private:
    Queue<PlayerData>&   save_queue;
    PersistenceMonitor& persistence_monitor;
    std::atomic<bool> vive;
};

#endif // PERSISTENCE_THREAD_H