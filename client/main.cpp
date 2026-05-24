#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <iostream>
#include <exception>

#include "game/GameLoop.h"

int main() try {
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

    GameLoop game_loop(window, renderer);
    game_loop.run();

    return 0;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
