#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>

class AudioConfig {
public:
    // Estructura para configuración del mixer
    struct Mixer {
        int frequency;
        int channels;
        int chunksize;
        int default_music_volume;
    };

    // Estructura para volúmenes de efectos
    struct EffectVolumes {
        int max_audible_tiles;
        int min_effect_volume;
        int max_effect_volume;
        uint32_t effect_cooldown_ms;
    };

    // Datos públicos
    Mixer mixer;
    EffectVolumes effect_volumes;

    // Datos de sonidos (mapas de clave -> ruta)
    std::unordered_map<std::string, std::string> combat_melee_sounds;
    std::unordered_map<std::string, std::string> combat_ranged_sounds;
    std::unordered_map<std::string, std::string> magic_sounds;
    std::unordered_map<std::string, std::string> death_sounds;
    std::unordered_map<std::string, std::string> creature_sounds;
    std::unordered_map<std::string, std::string> npc_merchant_sounds;
    std::unordered_map<std::string, std::string> npc_banker_sounds;
    std::unordered_map<std::string, std::string> npc_priest_sounds;

    // Datos de interacción
    struct Interaction {
        int shop_range_tiles;
        int priest_range_tiles;
    };
    Interaction interaction;

    // Singleton
    static AudioConfig& instance();

    // Cargar configuración desde TOML
    bool load(const std::string& config_path);

    // Getters auxiliares
    std::string get_combat_melee_sound(const std::string& weapon) const;
    std::string get_combat_ranged_sound(const std::string& weapon) const;
    std::string get_magic_sound(const std::string& spell_name) const;
    std::string get_creature_sound(const std::string& creature_type) const;

private:
    AudioConfig() = default;
    ~AudioConfig() = default;

    // Prevenir copia
    AudioConfig(const AudioConfig&) = delete;
    AudioConfig& operator=(const AudioConfig&) = delete;
};

#endif // AUDIO_CONFIG_H
