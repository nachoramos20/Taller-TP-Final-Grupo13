#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <atomic>
#include <memory>

#include "game/GameLoop.h"
#include "game/LoginScreen.h"
#include "audio/AudioManager.h"
#include "../common/socket.h"
#include "../common/queue.h"
#include "../common/protocol/dtos.h"
#include "../common/protocol/protocol.h"
#include "../common/MapaDTO.h"
#include "net/Command.h"
#include "net/SenderThread.h"
#include "net/ReceiverThread.h"
#include "config/ClientConfig.h"
#include "config/AudioConfig.h"
#include "config/SpellVfxConfig.h"
#include "config/RacesClassesConfig.h"
#include "render/ItemVisualConfig.h"
#include "render/NpcVisualConfig.h"

// Detiene y joinea todos los threads de red de forma segura.
static void shutdown_net(std::atomic<bool>& connected,
                         Queue<Command>& cq,
                         Queue<SnapshotDTO>& sq,
                         Queue<MapaDTO>& mq,
                         SenderThread& sender,
                         ReceiverThread& receiver) {
    connected = false;
    try { cq.close(); } catch (...) {}
    try { sq.close(); } catch (...) {}
    try { mq.close(); } catch (...) {}
    sender.stop();
    receiver.stop();
    sender.join();
    receiver.join();
}

int main(int argc, char* argv[]) try {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <puerto>\n";
        return 1;
    }
    const std::string host = argv[1];
    const std::string port = argv[2];

    // Cargar todas las configuraciones desde TOML
    ClientConfig& client_config = ClientConfig::instance();
    AudioConfig& audio_config = AudioConfig::instance();
    SpellVfxConfig& spell_vfx_config = SpellVfxConfig::instance();
    RacesClassesConfig& races_classes_config = RacesClassesConfig::instance();
    ItemVisualConfig& item_visual_config = ItemVisualConfig::instance();
    NpcVisualConfig& npc_visual_config = NpcVisualConfig::instance();

    if (!client_config.load("config/client_config.toml")) {
        std::cerr << "Error: no se pudo cargar config/client_config.toml\n";
        return 1;
    }
    if (!audio_config.load("config/audio_config.toml")) {
        std::cerr << "Error: no se pudo cargar config/audio_config.toml\n";
        return 1;
    }
    if (!spell_vfx_config.load("config/spells_vfx.toml")) {
        std::cerr << "Error: no se pudo cargar config/spells_vfx.toml\n";
        return 1;
    }
    if (!races_classes_config.load("config/races_classes.toml")) {
        std::cerr << "Error: no se pudo cargar config/races_classes.toml\n";
        return 1;
    }
    if (!item_visual_config.load("config/item_visuals.toml")) {
        std::cerr << "Error: no se pudo cargar config/item_visuals.toml\n";
        return 1;
    }
    if (!npc_visual_config.load("config/npc_visuals.toml")) {
        std::cerr << "Error: no se pudo cargar config/npc_visuals.toml\n";
        return 1;
    }

    SDL2pp::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init: " << TTF_GetError() << "\n";
        return 1;
    }

    AudioManager audio;
    audio.play_music_loop(client_config.music.main_theme_path);

    SDL2pp::Window window(
        client_config.ui.window_title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        client_config.ui.window_width, client_config.ui.window_height,
        SDL_WINDOW_RESIZABLE
    );
    SDL2pp::Renderer renderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    std::string pending_error;  // error del servidor para mostrar en el próximo login

    while (true) {
        // Mostrar pantalla de login/registro
        LoginScreen login_screen(window, renderer, client_config.fonts.chat_font_path);
        if (!pending_error.empty()) {
            login_screen.set_error(pending_error);
            pending_error.clear();
        }
        LoginResult result = login_screen.run();

        if (result.cancelled) break;  // salida limpia

        // Validación local
        if (result.username.empty()) { pending_error = "Ingresa un nombre."; continue; }
        if (result.username.size() > 20) { pending_error = "Nombre demasiado largo (max 20)."; continue; }
        for (char c : result.username) {
            if (c == ' ') { pending_error = "El nombre no puede tener espacios."; break; }
        }
        if (!pending_error.empty()) continue;

        // Conectar socket
        std::unique_ptr<Socket> sock;
        try {
            sock = std::make_unique<Socket>(host.c_str(), port.c_str());
        } catch (const std::exception& e) {
            pending_error = std::string("No se pudo conectar: ") + e.what();
            continue;
        }

        // Threads de red
        Queue<Command>     command_queue;
        Queue<SnapshotDTO> snapshot_queue;
        Queue<MapaDTO>     map_queue;
        std::atomic<bool>  connected(true);

        SenderThread   sender(*sock, command_queue);
        ReceiverThread receiver(*sock, snapshot_queue, map_queue, connected);
        sender.start();
        receiver.start();

        // Enviar handshake
        if (result.do_register)
            command_queue.push(Command::register_player(result.username, result.race, result.cls));
        else
            command_queue.push(Command::login(result.username));

        // Esperar respuesta del servidor
        std::string error_msg;
        HandshakeResult hs = receiver.wait_handshake(error_msg);

        if (hs != HandshakeResult::OK) {
            shutdown_net(connected, command_queue, snapshot_queue, map_queue, sender, receiver);
            pending_error = error_msg.empty()
                ? races_classes_config.get_login_messages().invalid_user
                : error_msg;
            continue;
        }

        // GameLoop
        try {
            GameLoop game_loop(window, renderer,
                               &command_queue, &snapshot_queue,
                               &map_queue, &connected, &audio);
            game_loop.run();
        } catch (const std::exception& e) {
            std::cerr << "GameLoop error: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "GameLoop: excepcion desconocida\n";
        }

        // Cleanup y salir
        shutdown_net(connected, command_queue, snapshot_queue, map_queue, sender, receiver);
        break;  // sesión terminada: no volver al login
    }

    TTF_Quit();
    return 0;

} catch (const std::exception& e) {
    std::cerr << "Error fatal: " << e.what() << "\n";
    TTF_Quit();
    return 1;
}
