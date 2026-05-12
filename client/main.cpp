#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>

#include <iostream>
#include <exception>

using namespace SDL2pp;

int main() try {
    // 1. Inicialización: SDL_INIT_VIDEO levanta video y eventos
    SDL sdl(SDL_INIT_VIDEO);

    // 2. Ventana
    Window window("Argentum - demo movimiento",
                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                  800, 600,
                  SDL_WINDOW_RESIZABLE);

    // 3. Renderer acelerado por GPU + vsync (limita a la tasa del monitor)
    Renderer renderer(window, -1,
                      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // 4. Estado del "jugador": posición y velocidad (pixels/segundo)
    float x = 400.0f;
    float y = 300.0f;
    const float velocidad = 200.0f;  // px/s

    // 5. Game loop
    bool running = true;
    Uint32 ticks_anterior = SDL_GetTicks();

    while (running) {
        // -- 5a. Calcular delta time --
        Uint32 ticks_actual = SDL_GetTicks();
        float dt = (ticks_actual - ticks_anterior) / 1000.0f;  // a segundos
        ticks_anterior = ticks_actual;

        // -- 5b. Procesar eventos --
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN
                       && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        // -- 5c. Estado del teclado: para movimiento continuo --
        // (distinto a SDL_KEYDOWN, que es solo el evento del momento que apretás)
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_LEFT])  x -= velocidad * dt;
        if (keys[SDL_SCANCODE_RIGHT]) x += velocidad * dt;
        if (keys[SDL_SCANCODE_UP])    y -= velocidad * dt;
        if (keys[SDL_SCANCODE_DOWN])  y += velocidad * dt;

        // -- 5d. Render --
        renderer.SetDrawColor(20, 20, 40, 255);  // fondo azul oscuro
        renderer.Clear();

        renderer.SetDrawColor(220, 60, 60, 255); // rojo
        Rect cuadrado(static_cast<int>(x) - 16,
                      static_cast<int>(y) - 16,
                      32, 32);
        renderer.FillRect(cuadrado);

        renderer.Present();  // muestra el frame; con vsync espera al monitor
    }

    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
