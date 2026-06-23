#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>

// Configuración del cliente cargada desde client_config.toml: rutas de
// assets, fuentes, ventana, atajos de teclado y los parámetros de
// rendering (tile_size, zonas seguras espejadas del server para VFX, etc.).
class ClientConfig {
public:
    struct AssetsPaths {
        std::string fonts_dir;
        std::string sounds_dir;
        std::string sprites_dir;
        std::string music_dir;
        std::string effects_dir;
        std::string creatures_dir;
    };

    struct Fonts {
        std::string default_path;
        int default_size = 0;
        std::string chat_font_path;
        int chat_font_size = 0;
        int title_font_size = 0;
        int medium_font_size = 0;
        int small_font_size = 0;
    };

    struct Music {
        std::string main_theme_path;
        float main_theme_volume = 1.0f;
    };

    struct Rendering {
        int tile_size = 0;
        int map_size = 0;
        int obj_sup_tiles = 0;
        int obj_sup_size = 0;
        int obj_sup_ticks_per_frame = 0;
        int spell_ticks_per_frame = 0;
        uint16_t water_floor_id = 0;
        int water_search_radius_tiles = 0;
        uint16_t grass_floor_id_min = 0;
        uint16_t grass_floor_id_max = 0;
        uint16_t city_stone_floor_id = 0;
        uint16_t dirt_floor_id = 0;
        uint16_t sand_floor_id = 0;

        // Zona del bosque (dos rectángulos con el mismo rango de X, uno al
        // norte y otro al sur), para sonido ambiente de fauna.
        int forest_x_min = 0;
        int forest_x_max = 0;
        int forest_y1_min = 0;
        int forest_y1_max = 0;
        int forest_y2_min = 0;
        int forest_y2_max = 0;

        // Zona del cementerio, para sonido ambiente de viento.
        int cemetery_x_min = 0;
        int cemetery_x_max = 0;
        int cemetery_y_min = 0;
        int cemetery_y_max = 0;

        // Zonas seguras (ciudad principal y pueblo sur), espejadas de
        // ServerGameLoop.cpp: para no spawnear el VFX de hechizos/proyectiles
        // cuando el servidor va a rechazar la acción por estar en zona segura.
        int safe_zone1_x_min = 0, safe_zone1_x_max = 0, safe_zone1_y_min = 0, safe_zone1_y_max = 0;
        int safe_zone2_x_min = 0, safe_zone2_x_max = 0, safe_zone2_y_min = 0, safe_zone2_y_max = 0;
    };

    // Se cargan por nombre desde el TOML (p. ej. "I", "Up", "Escape") y se
    // resuelven una sola vez a códigos SDL.
    struct Keybindings {
        SDL_Keycode toggle_position_label = SDLK_UNKNOWN;
        SDL_Keycode toggle_inventory = SDLK_UNKNOWN;
        SDL_Keycode drop_item = SDLK_UNKNOWN;
        SDL_Keycode meditate = SDLK_UNKNOWN;
        SDL_Keycode resurrect = SDLK_UNKNOWN;
        SDL_Keycode pick_item = SDLK_UNKNOWN;
        SDL_Keycode quit = SDLK_UNKNOWN;
        SDL_Keycode help = SDLK_UNKNOWN;

        std::vector<SDL_Scancode> move_up;
        std::vector<SDL_Scancode> move_down;
        std::vector<SDL_Scancode> move_left;
        std::vector<SDL_Scancode> move_right;
    };

    struct UI {
        std::string window_title;
        int window_width = 0;
        int window_height = 0;
        bool window_resizable = false;
        int window_min_height = 0;
    };

    struct DeathEffects {
        int death_frames = 0;
        uint32_t death_frame_ms = 0;
        uint32_t death_linger_ms = 0;
        uint32_t death_duration_ms = 0;
        std::string sprite_base_path;
    };

    struct Camera {
        int initial_x = 0;
        int initial_y = 0;
    };

    struct Projectiles {
        uint32_t duration_ticks = 0;
    };

    AssetsPaths assets;
    Fonts fonts;
    Music music;
    Rendering rendering;
    UI ui;
    DeathEffects death_effects;
    Camera camera;
    Projectiles projectiles;
    Keybindings keybindings;

    static ClientConfig& instance();

    bool load(const std::string& config_path);

    // Antes una función global suelta en PlayerState.h; es un simple
    // forwarder a rendering.tile_size, así que pasa a ser un método.
    int tile_size() const { return rendering.tile_size; }

private:
    ClientConfig() = default;
    ~ClientConfig() = default;

    ClientConfig(const ClientConfig&) = delete;
    ClientConfig& operator=(const ClientConfig&) = delete;
};

#endif  // CLIENT_CONFIG_H
