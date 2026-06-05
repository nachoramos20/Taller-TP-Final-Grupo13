#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <iostream>
#include <exception>
#include <string>
#include <atomic>

#include "game/GameLoop.h"
#include "../common/socket.h"
#include "../common/queue.h"
#include "../common/protocol/dtos.h"
#include "../common/protocol/protocol.h"
#include "../common/MapaDTO.h"
#include "net/Command.h"
#include "net/SenderThread.h"
#include "net/ReceiverThread.h"

static void print_menu() {
    std::cout << "\n=== Argentum Online - Grupo 13 ===\n";
    std::cout << "1. Iniciar sesion\n";
    std::cout << "2. Registrarse\n";
    std::cout << "Opcion: ";
}

static uint8_t select_race() {
    std::cout << "Raza:\n";
    std::cout << "  0 - Humano\n";
    std::cout << "  1 - Elfo\n";
    std::cout << "  2 - Enano\n";
    std::cout << "  3 - Gnomo\n";
    std::cout << "Opcion: ";
    int r; std::cin >> r;
    return static_cast<uint8_t>(r % 4);
}

static uint8_t select_class() {
    std::cout << "Clase:\n";
    std::cout << "  0 - Mago\n";
    std::cout << "  1 - Clerigo\n";
    std::cout << "  2 - Paladin\n";
    std::cout << "  3 - Guerrero\n";
    std::cout << "Opcion: ";
    int c; std::cin >> c;
    return static_cast<uint8_t>(c % 4);
}

int main(int argc, char* argv[]) try {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <puerto>\n";
        return 1;
    }

    std::string host = argv[1];
    std::string port = argv[2];

    // Conectar al servidor
    Socket socket(host.c_str(), port.c_str());

    // Colas
    Queue<Command>     command_queue;
    Queue<SnapshotDTO> snapshot_queue;
    Queue<MapaDTO>     map_queue;

    std::atomic<bool> connected(true);
    SenderThread      sender(socket, command_queue);
    ReceiverThread    receiver(socket, snapshot_queue, map_queue, connected);

    sender.start();
    receiver.start();

    // Login / Registro por consola 
    print_menu();
    int opcion; std::cin >> opcion;
    std::cin.ignore();

    std::string username;
    std::cout << "Usuario: ";
    std::getline(std::cin, username);

    if (opcion == 2) {
        uint8_t race = select_race();
        uint8_t cls  = select_class();
        command_queue.push(Command::register_player(username, race, cls));
    } else {
        command_queue.push(Command::login(username));
    }

    // SDL
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);

    SDL2pp::Window window(
        "Argentum Online - Grupo 13",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_RESIZABLE
    );

    SDL2pp::Renderer renderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    // GameLoop
    try {
        GameLoop game_loop(window, renderer,
                           &command_queue, &snapshot_queue,
                           &map_queue, &connected);
        game_loop.run();
    } catch (const std::exception& e) {
        std::cerr << "GameLoop error: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "GameLoop error: excepcion desconocida\n";
    }

    // Shutdown 
    connected = false;
    command_queue.close();
    snapshot_queue.close();
    map_queue.close();
    sender.stop();
    receiver.stop();
    sender.join();
    receiver.join();

    return 0;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}