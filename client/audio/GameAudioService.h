#pragma once

#include <cstdint>
#include <string>
#include <vector>

class AudioManager;

// Fachada de audio del cliente.
// Centraliza la traducción de eventos de juego a reproducciones concretas en
// AudioManager, resolviendo assets y variantes mediante la configuración.
// Esto evita que otras clases del cliente dependan de AudioManager o conozcan
// rutas de archivos de sonido.
//
// Si se construye con `AudioManager == nullptr`, todos los métodos se comportan
// como no-op.
class GameAudioService {
public:
    explicit GameAudioService(AudioManager* audio);

    void attack(uint8_t weapon_item, float dist_tiles);
    // Cuando un enemigo (NPC u otro jugador) golpea al personaje propio.
    void damage_received(float dist_tiles);
    void spell_cast(uint8_t spell_id, float dist_tiles);
    void npc_death(uint8_t npc_sprite_id, uint16_t entity_id, uint16_t pos_y,
                   float dist_tiles, uint8_t own_weapon_item);
    void player_death(float dist_tiles);

    void level_up();
    void coins_received();
    void player_spawn();
    void potion_used();

    void update_meditation_loop(bool meditating);

    void click();

    void clan_created();
    void clan_member_attacked();
    void private_message_received();

    void update_ocean_ambient(float dist_tiles);
    void update_cemetery_ambient(float dist_tiles);

    // Fauna del bosque (pajaritos, cuervos): suena cada tanto, a intervalos
    // aleatorios, mientras el jugador está dentro de la zona del bosque.
    // Pensado para llamarse todos los frames con la condición actual.
    void update_forest_ambience(bool in_forest);

    void footstep_grass();

    // Pasos en piedra de ciudad: audio largo en loop, suena mientras el
    // jugador camina sobre ese piso y se corta apenas se detiene o cambia
    // de piso. Pensado para llamarse todos los frames con la condición.
    void update_city_stone_footsteps(bool walking_on_city_stone);

    void merchant_greet(float dist_tiles);
    void merchant_farewell(float dist_tiles);
    void merchant_list_items(float dist_tiles);
    void merchant_buy(float dist_tiles);

    // El banquero no tiene despedida.
    void banker_greet(float dist_tiles);
    void banker_deposit(float dist_tiles);
    void banker_withdraw(float dist_tiles);

    void priest_greet(float dist_tiles);
    void priest_farewell(float dist_tiles);
    void priest_heal(float dist_tiles);
    void priest_resurrect(float dist_tiles);

private:
    void play_random(const std::vector<std::string>& paths, float dist_tiles, float volume_scale = 1.0f);
    void play_sequential(const std::vector<std::string>& paths, float dist_tiles, size_t& index, float volume_scale = 1.0f);
    void speak_sequence(const std::vector<std::string>& lines, float dist_tiles, uint32_t gap_ms = 200);
    void speak_random(const std::vector<std::string>& lines, float dist_tiles, uint32_t gap_ms = 200);

    AudioManager* _audio;
    size_t _footstep_grass_index = 0;
    uint32_t _next_forest_sound_ms = 0;
};
