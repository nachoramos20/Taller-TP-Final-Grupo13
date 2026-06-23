#include <iostream>
#include <string>
#include <vector>

#include "../common/queue.h"
#include "game/PersistenceMonitor.h"
#include "game/PersistenceThread.h"
#include "game/QueueMonitor.h"
#include "game/ServerGameLoop.h"
#include "game/config/GameConfig.h"
#include "game/world/MapaBuilder.h"
#include "net/Acceptor.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    try {
        std::string port = argv[1];

        GameConfig::get().load("config");

        Queue<std::shared_ptr<ServerCommand>> command_queue;
        QueueMonitor queue_monitor;
        Queue<PlayerData> save_queue;
        PersistenceMonitor persistence_monitor(save_queue);

        MapaBuilder mapa_builder;
        MapaDTO mapa = mapa_builder.build_mapa_inicial();
        std::vector<uint8_t> collision_map = mapa_builder.take_collision();

        Acceptor acceptor(port, command_queue, queue_monitor, persistence_monitor, mapa);
        ServerGameLoop game_loop(command_queue, queue_monitor, save_queue,
                                 std::move(collision_map));
        PersistenceThread persistence_thread(save_queue, persistence_monitor);

        std::cout << "Servidor escuchando en puerto " << port << "\n";
        std::cout << "Presioná 'q' + Enter para cerrar\n";

        persistence_thread.start();
        game_loop.start();
        acceptor.start();

        char c;
        while (std::cin >> c && c != 'q') {}

        std::cout << "Cerrando servidor...\n";
        acceptor.stop();
        acceptor.join();
        game_loop.stop();
        game_loop.join();
        persistence_thread.stop();
        persistence_thread.join();

    } catch (const std::exception& e) {
        std::cerr << "Error fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
