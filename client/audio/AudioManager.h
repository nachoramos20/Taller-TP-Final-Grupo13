#pragma once

#include <SDL_mixer.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Maneja música de fondo y efectos de sonido (SDL_mixer, API C directa).
// Debe vivir durante toda la ejecución del cliente: el dispositivo de audio
// se abre en el constructor y se cierra al destruirse.
class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Carga y reproduce una pista en loop infinito. Llamarlo de nuevo
    // reemplaza la música actual.
    void play_music_loop(const std::string& path);

    void set_music_volume(int volume);  // 0 (silencio) a MIX_MAX_VOLUME (128)

    // Reproduce un efecto puntual, con volumen modulado por la distancia en
    // tiles entre el evento y el jugador (más lejos = más bajo; más allá de
    // MAX_AUDIBLE_TILES no se reproduce). Tiene un cooldown por archivo para
    // no saturar de sonido cuando ocurren muchos eventos iguales a la vez.
    void play_effect_at(const std::string& path, float dist_tiles);

    // Igual que play_effect_at pero elige un archivo al azar de la lista
    // (para variantes de un mismo sonido, p. ej. voces de NPC).
    void play_random_effect_at(const std::vector<std::string>& paths, float dist_tiles);

    // Encola varias frases para que se reproduzcan una después de la otra
    // (no superpuestas), p. ej. el saludo de varias líneas de un NPC.
    // Hay que llamar a update() todos los frames para que avance la cola.
    void queue_speech_sequence(const std::vector<std::string>& paths, float dist_tiles);
    void update();

private:
    struct QueuedSpeech {
        std::string path;
        float dist_tiles;
        uint32_t fire_at_ms;
    };

    Mix_Chunk* load_chunk(const std::string& path);
    bool should_throttle(const std::string& path);
    void play_effect_now(const std::string& path, float dist_tiles);
    uint32_t chunk_duration_ms(Mix_Chunk* chunk) const;

    Mix_Music* _music = nullptr;
    std::unordered_map<std::string, Mix_Chunk*> _chunk_cache;
    std::unordered_map<std::string, uint32_t> _last_played_ms;
    std::vector<QueuedSpeech> _speech_queue;
};
