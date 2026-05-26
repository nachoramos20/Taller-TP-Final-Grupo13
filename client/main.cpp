#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <iostream>
#include <exception>
#include <string>

#include "game/GameLoop.h"
#include "../common/socket.h"
#include "../common/queue.h"
#include "../common/protocol/dtos.h"
#include "net/Command.h"
#include "net/SenderThread.h"
#include "net/ReceiverThread.h"

int main(int argc, char* argv[]) try {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <puerto>\n";
        return 1;
    }

    std::string host  = argv[1];
    std::string port  = argv[2];

    // Conectar al servidor
    Socket socket(host.c_str(), port.c_str());

    // Colas de comunicación
    Queue<Command>     command_queue;
    Queue<SnapshotDTO> snapshot_queue;

    // Hilos de red
    SenderThread   sender(socket, command_queue);
    std::atomic<bool> connected(true);
    ReceiverThread receiver(socket, snapshot_queue, connected);

    sender.start();
    receiver.start();

    // Mandar LOGIN
    command_queue.push(Command::login("jugador1"));

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

    // GameLoop con colas conectadas al servidor
    GameLoop game_loop(window, renderer, &command_queue, &snapshot_queue, &connected);
    game_loop.run();

    // Shutdown
    command_queue.close();
    snapshot_queue.close();
    sender.stop();
    receiver.stop();
    sender.join();
    receiver.join();

    return 0;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}