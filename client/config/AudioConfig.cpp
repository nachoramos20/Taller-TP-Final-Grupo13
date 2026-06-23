#include "AudioConfig.h"
#include <toml++/toml.h>
#include <iostream>

static void load_sound_table(toml::node_view<toml::node> table_view,
                              std::unordered_map<std::string, std::vector<std::string>>& target) {
    auto table = table_view.as_table();
    if (!table) return;
    for (auto& [key, val] : *table) {
        auto arr = val.as_array();
        if (!arr) continue;
        std::vector<std::string> sounds;
        for (auto& elem : *arr) {
            if (auto s = elem.as_string()) sounds.push_back(s->get());
        }
        target[std::string(key)] = std::move(sounds);
    }
}

AudioConfig& AudioConfig::instance() {
    static AudioConfig instance;
    return instance;
}

bool AudioConfig::load(const std::string& config_path) {
    try {
        auto config = toml::parse_file(config_path);

        if (auto mixer_table = config["mixer"]) {
            mixer.frequency = mixer_table["frequency"].value_or(22050);
            mixer.channels = mixer_table["channels"].value_or(2);
            mixer.chunksize = mixer_table["chunksize"].value_or(4096);
            mixer.default_music_volume = mixer_table["default_music_volume"].value_or(80);
        }

        if (auto effects_table = config["effect_volumes"]) {
            effect_volumes.max_audible_tiles = effects_table["max_audible_tiles"].value_or(15);
            effect_volumes.min_effect_volume = effects_table["min_effect_volume"].value_or(20);
            effect_volumes.max_effect_volume = effects_table["max_effect_volume"].value_or(128);
            effect_volumes.effect_cooldown_ms = effects_table["effect_cooldown_ms"].value_or(200);
        }

        load_sound_table(config["combat_sounds"]["melee"], combat_melee_sounds);
        load_sound_table(config["combat_sounds"]["ranged"], combat_ranged_sounds);
        load_sound_table(config["magic_sounds"]["spells"], magic_sounds);
        load_sound_table(config["death_sounds"], death_sounds);
        load_sound_table(config["creature_sounds"], creature_sounds);
        load_sound_table(config["npc_sounds"]["merchant"], npc_merchant_sounds);
        load_sound_table(config["npc_sounds"]["banker"], npc_banker_sounds);
        load_sound_table(config["npc_sounds"]["priest"], npc_priest_sounds);
        load_sound_table(config["economy_sounds"], economy_sounds);
        load_sound_table(config["ui_sounds"], ui_sounds);
        load_sound_table(config["ambient_sounds"], ambient_sounds);
        load_sound_table(config["movement_sounds"], movement_sounds);

        if (auto interaction_table = config["npc_interaction"]) {
            interaction.shop_range_tiles = interaction_table["shop_range_tiles"].value_or(3.0f);
            interaction.bank_range_tiles = interaction_table["bank_range_tiles"].value_or(3.0f);
            interaction.priest_range_tiles = interaction_table["priest_range_tiles"].value_or(3.0f);
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[AudioConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}

const std::vector<std::string>& AudioConfig::lookup(
        const std::unordered_map<std::string, std::vector<std::string>>& sounds,
        const std::string& key) {
    static const std::vector<std::string> EMPTY;
    auto it = sounds.find(key);
    return it != sounds.end() ? it->second : EMPTY;
}

const std::vector<std::string>& AudioConfig::get_combat_melee_sound(const std::string& weapon) const {
    auto it = combat_melee_sounds.find(weapon);
    if (it != combat_melee_sounds.end()) return it->second;
    return lookup(combat_melee_sounds, "generico");
}

const std::vector<std::string>& AudioConfig::get_combat_ranged_sound(const std::string& weapon) const {
    auto it = combat_ranged_sounds.find(weapon);
    if (it != combat_ranged_sounds.end()) return it->second;
    return lookup(combat_ranged_sounds, "flecha");
}

const std::vector<std::string>& AudioConfig::get_magic_sound(const std::string& spell_name) const {
    auto it = magic_sounds.find(spell_name);
    if (it != magic_sounds.end()) return it->second;
    return lookup(magic_sounds, "hechizo_generico");
}

const std::vector<std::string>& AudioConfig::get_creature_sound(const std::string& creature_type) const {
    return lookup(creature_sounds, creature_type);
}

const std::vector<std::string>& AudioConfig::get_death_sound(const std::string& key) const {
    return lookup(death_sounds, key);
}

const std::vector<std::string>& AudioConfig::get_npc_merchant_sound(const std::string& key) const {
    return lookup(npc_merchant_sounds, key);
}

const std::vector<std::string>& AudioConfig::get_npc_banker_sound(const std::string& key) const {
    return lookup(npc_banker_sounds, key);
}

const std::vector<std::string>& AudioConfig::get_npc_priest_sound(const std::string& key) const {
    return lookup(npc_priest_sounds, key);
}

const std::vector<std::string>& AudioConfig::get_economy_sound(const std::string& key) const {
    return lookup(economy_sounds, key);
}

const std::vector<std::string>& AudioConfig::get_ui_sound(const std::string& key) const {
    return lookup(ui_sounds, key);
}

const std::vector<std::string>& AudioConfig::get_ambient_sound(const std::string& key) const {
    return lookup(ambient_sounds, key);
}

const std::vector<std::string>& AudioConfig::get_movement_sound(const std::string& key) const {
    return lookup(movement_sounds, key);
}
