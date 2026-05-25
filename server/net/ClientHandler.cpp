#include "ClientHandler.h"
#include <sys/socket.h>

void ClientHandler::start() {
    _receiver.start();
}

void ClientHandler::stop() {
    _alive = false;
    _socket.shutdown(SHUT_RDWR);
    _snapshot_queue.close();
    _receiver.stop();
}

void ClientHandler::join() {
    _receiver.join();
}