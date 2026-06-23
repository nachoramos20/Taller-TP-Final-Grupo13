#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Configuración de audio cargada desde audio_config.toml: parámetros del
// mixer y, para cada categoría de sonido, las rutas posibles por clave
// (ver los getters get_*_sound, que elige el caller al azar entre ellas).
class AudioConfig {
public:
    struct Mixer {
        int frequency;
        int channels;
        int chunksize;
        int default_music_volume;
    };

    struct EffectVolumes {
        int max_audible_tiles;
        int min_effect_volume;
        int max_effect_volume;
        uint32_t effect_cooldown_ms;
    };

    Mixer mixer;
    EffectVolumes effect_volumes;

    std::unordered_map<std::string, std::vector<std::string>> combat_melee_sounds;
    std::unordered_map<std::string, std::vector<std::string>> combat_ranged_sounds;
    std::unordered_map<std::string, std::vector<std::string>> magic_sounds;
    std::unordered_map<std::string, std::vector<std::string>> death_sounds;
    std::unordered_map<std::string, std::vector<std::string>> creature_sounds;
    std::unordered_map<std::string, std::vector<std::string>> npc_merchant_sounds;
    std::unordered_map<std::string, std::vector<std::string>> npc_banker_sounds;
    std::unordered_map<std::string, std::vector<std::string>> npc_priest_sounds;
    std::unordered_map<std::string, std::vector<std::string>> economy_sounds;
    std::unordered_map<std::string, std::vector<std::string>> ui_sounds;
    std::unordered_map<std::string, std::vector<std::string>> ambient_sounds;
    std::unordered_map<std::string, std::vector<std::string>> movement_sounds;

    struct Interaction {
        float shop_range_tiles;
        float bank_range_tiles;
        float priest_range_tiles;
    };
    Interaction interaction;

    static AudioConfig& instance();

    bool load(const std::string& config_path);

    const std::vector<std::string>& get_combat_melee_sound(const std::string& weapon) const;
    const std::vector<std::string>& get_combat_ranged_sound(const std::string& weapon) const;
    const std::vector<std::string>& get_magic_sound(const std::string& spell_name) const;
    const std::vector<std::string>& get_creature_sound(const std::string& creature_type) const;
    const std::vector<std::string>& get_death_sound(const std::string& key) const;
    const std::vector<std::string>& get_npc_merchant_sound(const std::string& key) const;
    const std::vector<std::string>& get_npc_banker_sound(const std::string& key) const;
    const std::vector<std::string>& get_npc_priest_sound(const std::string& key) const;
    const std::vector<std::string>& get_economy_sound(const std::string& key) const;
    const std::vector<std::string>& get_ui_sound(const std::string& key) const;
    const std::vector<std::string>& get_ambient_sound(const std::string& key) const;
    const std::vector<std::string>& get_movement_sound(const std::string& key) const;

private:
    static const std::vector<std::string>& lookup(
            const std::unordered_map<std::string, std::vector<std::string>>& sounds,
            const std::string& key);

    AudioConfig() = default;
    ~AudioConfig() = default;

    AudioConfig(const AudioConfig&) = delete;
    AudioConfig& operator=(const AudioConfig&) = delete;
};

#endif  // AUDIO_CONFIG_H
