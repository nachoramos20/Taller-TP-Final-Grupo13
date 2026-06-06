#ifndef PERSISTENCE_THREAD_H
#define PERSISTENCE_THREAD_H


#include "../../common/thread.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/protocol/Serializer.h"
#include "../../common/protocol/dtos.h"
#include "PlayerData.h"
#include "PersistenceMonitor.h"

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