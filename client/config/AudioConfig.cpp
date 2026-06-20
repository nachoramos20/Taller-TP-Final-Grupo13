#include "AudioConfig.h"
#include <toml++/toml.h>
#include <iostream>

AudioConfig& AudioConfig::instance() {
    static AudioConfig instance;
    return instance;
}

bool AudioConfig::load(const std::string& config_path) {
    try {
        auto config = toml::parse_file(config_path);

        // Cargar mixer
        if (auto mixer_table = config["mixer"]) {
            mixer.frequency = mixer_table["frequency"].value_or(22050);
            mixer.channels = mixer_table["channels"].value_or(2);
            mixer.chunksize = mixer_table["chunksize"].value_or(4096);
            mixer.default_music_volume = mixer_table["default_music_volume"].value_or(80);
        }

        // Cargar effect_volumes
        if (auto effects_table = config["effect_volumes"]) {
            effect_volumes.max_audible_tiles = effects_table["max_audible_tiles"].value_or(15);
            effect_volumes.min_effect_volume = effects_table["min_effect_volume"].value_or(20);
            effect_volumes.max_effect_volume = effects_table["max_effect_volume"].value_or(128);
            effect_volumes.effect_cooldown_ms = effects_table["effect_cooldown_ms"].value_or(200);
        }

        // Cargar combat_sounds.melee
        if (auto melee_table = config["combat_sounds"]["melee"].as_table()) {
            for (auto& [key, val] : *melee_table) {
                if (auto str_val = val.as_string()) {
                    combat_melee_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar combat_sounds.ranged
        if (auto ranged_table = config["combat_sounds"]["ranged"].as_table()) {
            for (auto& [key, val] : *ranged_table) {
                if (auto str_val = val.as_string()) {
                    combat_ranged_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar magic_sounds.spells
        if (auto magic_table = config["magic_sounds"]["spells"].as_table()) {
            for (auto& [key, val] : *magic_table) {
                if (auto str_val = val.as_string()) {
                    magic_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar death_sounds
        if (auto death_table = config["death_sounds"].as_table()) {
            for (auto& [key, val] : *death_table) {
                if (auto str_val = val.as_string()) {
                    death_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar creature_sounds
        if (auto creature_table = config["creature_sounds"].as_table()) {
            for (auto& [key, val] : *creature_table) {
                if (auto str_val = val.as_string()) {
                    creature_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar npc_sounds.merchant
        if (auto merchant_table = config["npc_sounds"]["merchant"].as_table()) {
            for (auto& [key, val] : *merchant_table) {
                if (auto str_val = val.as_string()) {
                    npc_merchant_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar npc_sounds.banker
        if (auto banker_table = config["npc_sounds"]["banker"].as_table()) {
            for (auto& [key, val] : *banker_table) {
                if (auto str_val = val.as_string()) {
                    npc_banker_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar npc_sounds.priest
        if (auto priest_table = config["npc_sounds"]["priest"].as_table()) {
            for (auto& [key, val] : *priest_table) {
                if (auto str_val = val.as_string()) {
                    npc_priest_sounds[std::string(key)] = str_val->get();
                }
            }
        }

        // Cargar interaction
        if (auto interaction_table = config["npc_interaction"]) {
            interaction.shop_range_tiles = interaction_table["shop_range_tiles"].value_or(3);
            interaction.priest_range_tiles = interaction_table["priest_range_tiles"].value_or(3);
        }

        std::cout << "[AudioConfig] Configuración cargada exitosamente desde: " << config_path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[AudioConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}

std::string AudioConfig::get_combat_melee_sound(const std::string& weapon) const {
    auto it = combat_melee_sounds.find(weapon);
    if (it != combat_melee_sounds.end()) {
        return it->second;
    }
    return combat_melee_sounds.at("generico");
}

std::string AudioConfig::get_combat_ranged_sound(const std::string& weapon) const {
    auto it = combat_ranged_sounds.find(weapon);
    if (it != combat_ranged_sounds.end()) {
        return it->second;
    }
    return combat_ranged_sounds.at("flecha");
}

std::string AudioConfig::get_magic_sound(const std::string& spell_name) const {
    auto it = magic_sounds.find(spell_name);
    if (it != magic_sounds.end()) {
        return it->second;
    }
    return magic_sounds.at("hechizo_generico");
}

std::string AudioConfig::get_creature_sound(const std::string& creature_type) const {
    auto it = creature_sounds.find(creature_type);
    if (it != creature_sounds.end()) {
        return it->second;
    }
    return "";
}
