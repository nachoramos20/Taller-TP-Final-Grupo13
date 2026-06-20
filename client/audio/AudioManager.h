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
    // (para variantes de un mismo sonido, p. ej. sonidos de NPC al morir).
    void play_random_effect_at(const std::vector<std::string>& paths, float dist_tiles);

    // Diálogo de NPC: corta de inmediato cualquier frase que esté sonando o
    // pendiente (saludo, despedida, etc.) y empieza esta secuencia ya mismo.
    // Usa un canal de audio reservado, así nunca pisa efectos de combate u
    // otros sonidos. Hay que llamar a update() todos los frames para que las
    // frases siguientes de la secuencia se vayan disparando en orden.
    // gap_ms es la pausa entre una frase y la siguiente dentro de la secuencia.
    void speak(const std::vector<std::string>& paths, float dist_tiles, uint32_t gap_ms = 200);

    // Igual que speak pero elige un archivo al azar de la lista (para una
    // sola frase con variantes, p. ej. "lo pides lo tienes").
    void speak_random(const std::vector<std::string>& paths, float dist_tiles, uint32_t gap_ms = 200);

    void update();

private:
    struct QueuedSpeech {
        std::string path;
        float dist_tiles;
        uint32_t fire_at_ms;
    };

    Mix_Chunk* load_chunk(const std::string& path);
    Mix_Chunk* load_speech_chunk(const std::string& path);
    Mix_Chunk* trim_silence(Mix_Chunk* chunk) const;
    bool should_throttle(const std::string& path);
    void play_effect_now(const std::string& path, float dist_tiles);
    void play_speech_now(const std::string& path, float dist_tiles);
    uint32_t chunk_duration_ms(Mix_Chunk* chunk) const;

    Mix_Music* _music = nullptr;
    std::unordered_map<std::string, Mix_Chunk*> _chunk_cache;
    std::unordered_map<std::string, Mix_Chunk*> _speech_chunk_cache;  // version recortada (sin silencios)
    std::unordered_map<std::string, uint32_t> _last_played_ms;
    std::vector<QueuedSpeech> _speech_queue;  // resto de la secuencia actual, pendiente
};
