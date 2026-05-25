#include "ClientHandler.h"
#include <sys/socket.h>

void ClientHandler::start() {
    // TODO: arrancar ReceiverThread y SenderThread
}

void ClientHandler::stop() {
    _alive = false;
    _socket.shutdown(SHUT_RDWR);
    _snapshot_queue.close();
}

void ClientHandler::join() {
    // TODO: join de ReceiverThread y SenderThread
}