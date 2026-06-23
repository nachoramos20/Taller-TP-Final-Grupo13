#include <atomic>
#include <iostream>
#include <memory>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/socket.h"
#include "audio/AudioManager.h"
#include "config/ClientConfig.h"
#include "config/ConfigLoader.h"
#include "config/RacesClassesConfig.h"
#include "net/NetSession.h"
#include "ui/LoginScreen.h"
#include "ui/StatsPanel.h"

#include "GameLoop.h"

int main(int argc, char* argv[]) try {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <puerto>\n";
        return 1;
    }
    std::string host = argv[1];
    std::string port = argv[2];

    try {
        Socket test_sock(host.c_str(), port.c_str());
    } catch (const std::exception& e) {
        std::cerr << "No se pudo conectar al servidor en " << host << ":" << port << " ("
                  << e.what() << ")\n";
        return 1;
    }

    ClientConfig& client_config = ClientConfig::instance();
    RacesClassesConfig& races_classes_config = RacesClassesConfig::instance();

    std::string config_error;
    if (!ConfigLoader::load_all(config_error)) {
        std::cerr << config_error << "\n";
        return 1;
    }

    SDL2pp::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init: " << TTF_GetError() << "\n";
        return 1;
    }

    AudioManager audio;
    audio.play_music_loop(client_config.music.main_theme_path);

    SDL2pp::Window window(client_config.ui.window_title, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED, client_config.ui.window_width,
                          client_config.ui.window_height, SDL_WINDOW_RESIZABLE);
    SDL_SetWindowMinimumSize(window.Get(), StatsPanel::PANEL_W * 2,
                             client_config.ui.window_min_height);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    std::string pending_error;

    while (true) {
        LoginScreen login_screen(window, renderer, client_config.fonts.chat_font_path, &audio);
        if (!pending_error.empty()) {
            login_screen.set_error(pending_error);
            pending_error.clear();
        }
        LoginResult result = login_screen.run();

        if (result.cancelled)
            break;

        if (result.username.empty()) {
            pending_error = "Ingresa un nombre.";
            continue;
        }
        if (result.username.size() > 20) {
            pending_error = "Nombre demasiado largo (max 20).";
            continue;
        }
        for (char c: result.username) {
            if (c == ' ') {
                pending_error = "El nombre no puede tener espacios.";
                break;
            }
        }
        if (!pending_error.empty())
            continue;

        NetSession net_session;
        std::string connection_error;
        if (!net_session.connect(host, port, connection_error)) {
            pending_error = connection_error;
            continue;
        }

        std::string handshake_error;
        if (!net_session.authenticate(result.username, result.do_register, result.race, result.cls,
                                      handshake_error)) {
            pending_error = handshake_error.empty() ?
                                    races_classes_config.get_login_messages().invalid_user :
                                    handshake_error;
            continue;
        }

        try {
            GameLoop game_loop(window, renderer, &net_session.commands(), &net_session.snapshots(),
                               &net_session.maps(), &net_session.connection_state(), &audio,
                               result.username);
            game_loop.run();
        } catch (const std::exception& e) {
            std::cerr << "GameLoop error: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "GameLoop: excepcion desconocida\n";
        }

        net_session.shutdown();
        break;
    }

    TTF_Quit();
    return 0;

} catch (const std::exception& e) {
    std::cerr << "Error fatal: " << e.what() << "\n";
    TTF_Quit();
    return 1;
}
