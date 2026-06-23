#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL_mixer.h>

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
    // volume_scale (0..1) multiplica el volumen resultante, para sonidos que
    // deben sonar más bajo que el resto (p. ej. pasos).
    void play_effect_at(const std::string& path, float dist_tiles, float volume_scale = 1.0f);

    // Igual que play_effect_at pero elige un archivo al azar de la lista
    // (para variantes de un mismo sonido, p. ej. sonidos de NPC al morir).
    void play_random_effect_at(const std::vector<std::string>& paths, float dist_tiles,
                               float volume_scale = 1.0f);

    // Diálogo de NPC: corta de inmediato cualquier frase que esté sonando o
    // pendiente (saludo, despedida, etc.) y empieza esta secuencia ya mismo.
    // Usa un canal de audio reservado, así nunca pisa efectos de combate u
    // otros sonidos. Hay que llamar a update() todos los frames para que las
    // frases siguientes de la secuencia se vayan disparando en orden.
    // gap_ms es la pausa entre una frase y la siguiente dentro de la secuencia.
    void speak(const std::vector<std::string>& paths, float dist_tiles, uint32_t gap_ms = 200);

    // Igual que speak pero elige un archivo al azar de la lista (para una
    // sola frase con variantes, p. ej. "lo pides lo tienes").
    void speak_random(const std::vector<std::string>& paths, float dist_tiles,
                      uint32_t gap_ms = 200);

    // Sonido ambiente continuo (p. ej. olas del mar): arranca en loop si no
    // estaba sonando, ajusta el volumen según la distancia cada vez que se
    // llama (así se escucha más fuerte a medida que el jugador se acerca) y
    // lo corta solo si dist_tiles supera el rango audible. Pensado para
    // llamarse todos los frames con la distancia actual al punto de interés.
    void set_ambient_loop(const std::string& path, float dist_tiles);

    // Igual que set_ambient_loop pero en un canal reservado propio, para que
    // pueda sonar a la vez que el ambiente principal sin pisarlo.
    // max_audible_tiles permite que este sonido tenga un rango de audibilidad
    // más corto que el resto (p. ej. algo que debe sentirse localizado).
    void set_secondary_ambient_loop(const std::string& path, float dist_tiles,
                                    float max_audible_tiles = -1.0f);

    // Sonido en loop gateado por una condición booleana. Usa un canal reservado propio, distinto
    // del ambiente y del de diálogo. Pensado para llamarse todos los frames con la condición
    // actual.
    void set_looping_while(const std::string& path, bool active, float volume_scale = 1.0f);

    // Loop de meditación con solapamiento: en vez de cortar y reiniciar en
    // seco (lo que se siente como un corte si el audio tiene un fade-out al
    // final), alterna entre dos canales y arranca la siguiente vuelta un
    // poco antes de que termine la actual, así la entrada de la nueva tapa
    // la cola de la anterior. Pensado para llamarse con el estado actual
    // cada vez que llega un snapshot; update() encadena las vueltas.
    void set_meditation_loop(const std::string& path, bool active, float volume_scale = 1.0f);

    void update();

private:
    struct QueuedSpeech {
        std::string path;
        float dist_tiles;
        uint32_t fire_at_ms;
    };

    Mix_Chunk* load_chunk(const std::string& path);
    Mix_Chunk* load_speech_chunk(const std::string& path);
    Mix_Chunk* load_ambient_chunk(const std::string& path);
    Mix_Chunk* trim_silence(Mix_Chunk* chunk, uint32_t lead_pad_ms, uint32_t tail_pad_ms) const;
    bool should_throttle(const std::string& path);
    void play_effect_now(const std::string& path, float dist_tiles, float volume_scale = 1.0f);
    void play_speech_now(const std::string& path, float dist_tiles);
    uint32_t chunk_duration_ms(Mix_Chunk* chunk) const;
    void set_ambient_loop_on(int channel, std::string& tracked_path, const std::string& path,
                             float dist_tiles, float max_audible_tiles);
    void set_looping_while_on(int channel, std::string& tracked_path, const std::string& path,
                              bool active, float volume_scale);

    Mix_Music* _music = nullptr;
    std::unordered_map<std::string, Mix_Chunk*> _chunk_cache;
    std::unordered_map<std::string, Mix_Chunk*> _speech_chunk_cache;
    std::unordered_map<std::string, Mix_Chunk*> _ambient_chunk_cache;
    std::unordered_map<std::string, uint32_t> _last_played_ms;
    std::vector<QueuedSpeech> _speech_queue;  // resto de la secuencia actual, pendiente
    std::string _ambient_path;            // qué está sonando en el canal ambiente (vacío = nada)
    std::string _secondary_ambient_path;  // qué está sonando en el canal ambiente secundario
    std::string _looping_path;  // qué está sonando en el canal de loop gateado (vacío = nada)

    // Loop de meditación con solapamiento entre dos canales (ver set_meditation_loop)
    std::string _meditation_path;
    int _meditation_active_channel = -1;  // -1 = inactivo
    uint32_t _meditation_next_switch_ms = 0;
    float _meditation_volume_scale = 1.0f;
};
