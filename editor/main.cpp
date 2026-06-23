#include <exception>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

int main() try {
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);

    SDL2pp::Window window("Argentum Online - Editor", SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_RESIZABLE);

    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }

        renderer.SetDrawColor(40, 40, 40, 255);
        renderer.Clear();
        renderer.Present();
    }

    return 0;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}