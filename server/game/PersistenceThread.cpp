#include "PersistenceThread.h"

PersistenceThread::PersistenceThread(Queue<PlayerData> &save_queue, PersistenceMonitor &persistence_monitor)
: save_queue (save_queue), persistence_monitor(persistence_monitor), vive(true){}

void PersistenceThread::run()
{
    try {
        while (vive) {
            PlayerData data_to_save = save_queue.pop();
            persistence_monitor.save_player(data_to_save);
        }
    }
    catch (const ClosedQueue&) {
    } catch (const std::exception& e) {
        std::cerr << "Error en hilo de persistencia: " << e.what() << "\n";
    }
}

void PersistenceThread::stop() {
    this->vive = false;
    this->save_queue.close();
    Thread::stop();
}
