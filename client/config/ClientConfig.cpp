#include "ClientConfig.h"
#include <toml++/toml.hpp>
#include <iostream>

ClientConfig& ClientConfig::instance() {
    static ClientConfig instance;
    return instance;
}

bool ClientConfig::load(const std::string& config_path) {
    try {
        auto config = toml::parse_file(config_path);

        // Cargar assets_paths
        if (auto assets_table = config["assets_paths"]) {
            assets.fonts_dir = assets_table["fonts_dir"].value_or(std::string("assets/fonts"));
            assets.sounds_dir = assets_table["sounds_dir"].value_or(std::string("assets/sounds"));
            assets.sprites_dir = assets_table["sprites_dir"].value_or(std::string("assets/sprites"));
            assets.music_dir = assets_table["music_dir"].value_or(std::string("assets/music"));
            assets.effects_dir = assets_table["effects_dir"].value_or(std::string("assets/effects"));
            assets.creatures_dir = assets_table["creatures_dir"].value_or(std::string("assets/creatures"));
        }

        // Cargar fonts
        if (auto fonts_table = config["fonts"]) {
            fonts.default_path = fonts_table["default_path"].value_or(std::string(""));
            fonts.default_size = fonts_table["default_size"].value_or(12);
            fonts.chat_font_path = fonts_table["chat_font_path"].value_or(std::string(""));
            fonts.chat_font_size = fonts_table["chat_font_size"].value_or(12);
            fonts.npc_font_path = fonts_table["npc_font_path"].value_or(std::string(""));
            fonts.npc_font_size = fonts_table["npc_font_size"].value_or(11);
            fonts.login_font_path = fonts_table["login_font_path"].value_or(std::string(""));
            fonts.login_font_size = fonts_table["login_font_size"].value_or(14);
        }

        // Cargar music
        if (auto music_table = config["music"]) {
            music.main_theme_path = music_table["main_theme_path"].value_or(std::string(""));
            music.main_theme_volume = music_table["main_theme_volume"].value_or(0.8f);
        }

        // Cargar rendering
        if (auto rendering_table = config["rendering"]) {
            rendering.tile_size = rendering_table["tile_size"].value_or(32);
            rendering.map_size = rendering_table["map_size"].value_or(100);
            rendering.obj_sup_tiles = rendering_table["obj_sup_tiles"].value_or(10);
            rendering.obj_sup_size = rendering_table["obj_sup_size"].value_or(20);
            rendering.obj_sup_ticks_per_frame = rendering_table["obj_sup_ticks_per_frame"].value_or(5);
            rendering.spell_ticks_per_frame = rendering_table["spell_ticks_per_frame"].value_or(4);
        }

        // Cargar UI
        if (auto ui_table = config["ui"]) {
            ui.window_title = ui_table["window_title"].value_or(std::string("Argentum Online"));
            ui.window_width = ui_table["window_width"].value_or(1024);
            ui.window_height = ui_table["window_height"].value_or(768);
            ui.window_resizable = ui_table["window_resizable"].value_or(true);
        }

        // Cargar death_effects
        if (auto death_table = config["death_effects"]) {
            death_effects.death_frames = death_table["death_frames"].value_or(6);
            death_effects.death_frame_ms = death_table["death_frame_ms"].value_or(150);
            death_effects.death_linger_ms = death_table["death_linger_ms"].value_or(2000);
            death_effects.death_duration_ms = death_table["death_duration_ms"].value_or(2900);
        }

        // Cargar camera
        if (auto camera_table = config["camera"]) {
            camera.initial_x = camera_table["initial_x"].value_or(0);
            camera.initial_y = camera_table["initial_y"].value_or(0);
        }

        // Cargar projectiles
        if (auto projectiles_table = config["projectiles"]) {
            projectiles.duration_ticks = projectiles_table["duration_ticks"].value_or(30);
        }

        std::cout << "[ClientConfig] Configuración cargada exitosamente desde: " << config_path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ClientConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}
