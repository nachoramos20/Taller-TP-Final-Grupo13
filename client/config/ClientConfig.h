#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class ClientConfig {
public:
    // Estructura para rutas de assets
    struct AssetsPaths {
        std::string fonts_dir;
        std::string sounds_dir;
        std::string sprites_dir;
        std::string music_dir;
        std::string effects_dir;
        std::string creatures_dir;
    };

    // Estructura para fuentes
    struct Fonts {
        std::string default_path;
        int default_size;
        std::string chat_font_path;
        int chat_font_size;
        int title_font_size;
        int medium_font_size;
        int small_font_size;
    };

    // Estructura para música
    struct Music {
        std::string main_theme_path;
        float main_theme_volume;
    };

    // Estructura para rendering
    struct Rendering {
        int tile_size;
        int map_size;
        int obj_sup_tiles;
        int obj_sup_size;
        int obj_sup_ticks_per_frame;
        int spell_ticks_per_frame;
        uint16_t water_floor_id;
        int water_search_radius_tiles;
        uint16_t grass_floor_id_min;
        uint16_t grass_floor_id_max;
        uint16_t city_stone_floor_id;
        uint16_t dirt_floor_id;
        uint16_t sand_floor_id;

        // Zona del bosque (dos rectángulos con el mismo rango de X, uno al
        // norte y otro al sur), para sonido ambiente de fauna.
        int forest_x_min;
        int forest_x_max;
        int forest_y1_min;
        int forest_y1_max;
        int forest_y2_min;
        int forest_y2_max;

        // Zona del cementerio, para sonido ambiente de viento.
        int cemetery_x_min;
        int cemetery_x_max;
        int cemetery_y_min;
        int cemetery_y_max;

        // Zonas seguras (ciudad principal y pueblo sur), espejadas de
        // ServerGameLoop.cpp: para no spawnear el VFX de hechizos/proyectiles
        // cuando el servidor va a rechazar la acción por estar en zona segura.
        int safe_zone1_x_min, safe_zone1_x_max, safe_zone1_y_min, safe_zone1_y_max;
        int safe_zone2_x_min, safe_zone2_x_max, safe_zone2_y_min, safe_zone2_y_max;
    };

    // Estructura para atajos de teclado (se cargan por nombre desde el TOML,
    // p. ej. "I", "Up", "Escape", y se resuelven una sola vez a códigos SDL).
    struct Keybindings {
        SDL_Keycode toggle_position_label = SDLK_UNKNOWN;
        SDL_Keycode toggle_inventory       = SDLK_UNKNOWN;
        SDL_Keycode drop_item              = SDLK_UNKNOWN;
        SDL_Keycode meditate               = SDLK_UNKNOWN;
        SDL_Keycode resurrect              = SDLK_UNKNOWN;
        SDL_Keycode pick_item              = SDLK_UNKNOWN;
        SDL_Keycode quit                   = SDLK_UNKNOWN;
        SDL_Keycode help                   = SDLK_UNKNOWN;

        std::vector<SDL_Scancode> move_up;
        std::vector<SDL_Scancode> move_down;
        std::vector<SDL_Scancode> move_left;
        std::vector<SDL_Scancode> move_right;
    };

    // Estructura para UI
    struct UI {
        std::string window_title;
        int window_width;
        int window_height;
        bool window_resizable;
        int window_min_height;
    };

    // Estructura para death effects
    struct DeathEffects {
        int death_frames;
        uint32_t death_frame_ms;
        uint32_t death_linger_ms;
        uint32_t death_duration_ms;
        std::string sprite_base_path; 
    };

    // Estructura para camera
    struct Camera {
        int initial_x;
        int initial_y;
    };

    // Estructura para projectiles
    struct Projectiles {
        uint32_t duration_ticks;
    };

    // Datos públicos
    AssetsPaths assets;
    Fonts fonts;
    Music music;
    Rendering rendering;
    UI ui;
    DeathEffects death_effects;
    Camera camera;
    Projectiles projectiles;
    Keybindings keybindings;

    // Singleton
    static ClientConfig& instance();

    // Cargar configuración desde TOML
    bool load(const std::string& config_path);

private:
    ClientConfig() = default;
    ~ClientConfig() = default;

    // Prevenir copia
    ClientConfig(const ClientConfig&) = delete;
    ClientConfig& operator=(const ClientConfig&) = delete;
};

#endif // CLIENT_CONFIG_H
