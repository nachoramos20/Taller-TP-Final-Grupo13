#include <iostream>
#include <string>
#include "net/Acceptor.h"
#include "game/ServerGameLoop.h"
#include "game/QueueMonitor.h"
#include "game/PersistenceMonitor.h"
#include "../common/queue.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    try {
        std::string port = argv[1];

        Queue<std::shared_ptr<ServerCommand>> command_queue;
        QueueMonitor queue_monitor;
        Queue<PlayerData> save_queue;
        PersistenceMonitor persistence_monitor(save_queue);

        Acceptor       acceptor(port, command_queue, queue_monitor, persistence_monitor);
        ServerGameLoop game_loop(command_queue, queue_monitor, save_queue);

        std::cout << "Servidor escuchando en puerto " << port << "\n";
        std::cout << "Presioná 'q' + Enter para cerrar\n";

        game_loop.start();
        acceptor.start();

        char c;
        while (std::cin >> c && c != 'q') {}

        std::cout << "Cerrando servidor...\n";
        acceptor.stop();
        acceptor.join();
        game_loop.stop();
        game_loop.join();

    } catch (const std::exception& e) {
        std::cerr << "Error fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}