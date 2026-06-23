#include "ClientConfig.h"
#include <toml++/toml.hpp>
#include <iostream>
#include <SDL2/SDL_keyboard.h>

static SDL_Keycode parse_key(toml::node_view<toml::node> node, const char* default_name) {
    std::string name = node.value_or(std::string(default_name));
    return SDL_GetKeyFromName(name.c_str());
}

static std::vector<SDL_Scancode> parse_scancodes(toml::node_view<toml::node> node) {
    std::vector<SDL_Scancode> result;
    if (auto arr = node.as_array()) {
        for (auto& elem : *arr) {
            if (auto name = elem.value<std::string>())
                result.push_back(SDL_GetScancodeFromName(name->c_str()));
        }
    }
    return result;
}

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
            fonts.title_font_size = fonts_table["title_font_size"].value_or(22);
            fonts.medium_font_size = fonts_table["medium_font_size"].value_or(14);
            fonts.small_font_size = fonts_table["small_font_size"].value_or(11);
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
            rendering.obj_sup_ticks_per_frame = rendering_table["obj_sup_ticks_per_frame"].value_or(8);
            rendering.spell_ticks_per_frame = rendering_table["spell_ticks_per_frame"].value_or(4);
            rendering.water_floor_id = rendering_table["water_floor_id"].value_or<uint16_t>(44);
            rendering.water_search_radius_tiles = rendering_table["water_search_radius_tiles"].value_or(18);
            rendering.grass_floor_id_min = rendering_table["grass_floor_id_min"].value_or<uint16_t>(2);
            rendering.grass_floor_id_max = rendering_table["grass_floor_id_max"].value_or<uint16_t>(9);
            rendering.city_stone_floor_id = rendering_table["city_stone_floor_id"].value_or<uint16_t>(1);
            rendering.dirt_floor_id = rendering_table["dirt_floor_id"].value_or<uint16_t>(10);
            rendering.forest_x_min = rendering_table["forest_x_min"].value_or(4);
            rendering.forest_x_max = rendering_table["forest_x_max"].value_or(22);
            rendering.forest_y1_min = rendering_table["forest_y1_min"].value_or(5);
            rendering.forest_y1_max = rendering_table["forest_y1_max"].value_or(34);
            rendering.forest_y2_min = rendering_table["forest_y2_min"].value_or(55);
            rendering.forest_y2_max = rendering_table["forest_y2_max"].value_or(95);
            rendering.cemetery_x_min = rendering_table["cemetery_x_min"].value_or(55);
            rendering.cemetery_x_max = rendering_table["cemetery_x_max"].value_or(66);
            rendering.cemetery_y_min = rendering_table["cemetery_y_min"].value_or(55);
            rendering.cemetery_y_max = rendering_table["cemetery_y_max"].value_or(66);
            rendering.safe_zone1_x_min = rendering_table["safe_zone1_x_min"].value_or(24);
            rendering.safe_zone1_x_max = rendering_table["safe_zone1_x_max"].value_or(56);
            rendering.safe_zone1_y_min = rendering_table["safe_zone1_y_min"].value_or(4);
            rendering.safe_zone1_y_max = rendering_table["safe_zone1_y_max"].value_or(36);
            rendering.safe_zone2_x_min = rendering_table["safe_zone2_x_min"].value_or(29);
            rendering.safe_zone2_x_max = rendering_table["safe_zone2_x_max"].value_or(49);
            rendering.safe_zone2_y_min = rendering_table["safe_zone2_y_min"].value_or(53);
            rendering.safe_zone2_y_max = rendering_table["safe_zone2_y_max"].value_or(71);
        }

        // Cargar UI
        if (auto ui_table = config["ui"]) {
            ui.window_title = ui_table["window_title"].value_or(std::string("Argentum Online"));
            ui.window_width = ui_table["window_width"].value_or(1024);
            ui.window_height = ui_table["window_height"].value_or(768);
            ui.window_resizable = ui_table["window_resizable"].value_or(true);
            ui.window_min_height = ui_table["window_min_height"].value_or(ui.window_height);
        }

        // Cargar death_effects
        if (auto death_table = config["death_effects"]) {
            death_effects.death_frames = death_table["death_frames"].value_or(6);
            death_effects.death_frame_ms = death_table["death_frame_ms"].value_or(150);
            death_effects.death_linger_ms = death_table["death_linger_ms"].value_or(2000);
            death_effects.death_duration_ms = death_table["death_duration_ms"].value_or(2900);
            death_effects.sprite_base_path = death_table["sprite_base_path"]
                .value_or(std::string("assets/sprites/stage/sangre_"));
        }

        // Cargar camera
        if (auto camera_table = config["camera"]) {
            camera.initial_x = camera_table["initial_x"].value_or(0);
            camera.initial_y = camera_table["initial_y"].value_or(0);
        }

        // Cargar projectiles
        if (auto rendering_table = config["rendering"]) {
            projectiles.duration_ticks = rendering_table["projectile_duration_ticks"].value_or(30);
        }

        // Cargar keybindings
        if (auto kb_table = config["keybindings"]) {
            keybindings.toggle_position_label = parse_key(kb_table["toggle_position_label"], "X");
            keybindings.toggle_inventory      = parse_key(kb_table["toggle_inventory"], "Tab");
            keybindings.drop_item             = parse_key(kb_table["drop_item"], "Q");
            keybindings.meditate              = parse_key(kb_table["meditate"], "M");
            keybindings.resurrect             = parse_key(kb_table["resurrect"], "R");
            keybindings.pick_item             = parse_key(kb_table["pick_item"], "E");
            keybindings.quit                  = parse_key(kb_table["quit"], "Escape");
            keybindings.help                  = parse_key(kb_table["help"], "H");

            keybindings.move_up    = parse_scancodes(kb_table["move_up"]);
            keybindings.move_down  = parse_scancodes(kb_table["move_down"]);
            keybindings.move_left  = parse_scancodes(kb_table["move_left"]);
            keybindings.move_right = parse_scancodes(kb_table["move_right"]);
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ClientConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}
