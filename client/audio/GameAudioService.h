#pragma once

#include <cstdint>
#include <string>
#include <vector>

class AudioManager;

// Único punto de acceso a sonido del juego (patrón Facade): traduce eventos
// de dominio (atacar, lanzar hechizo, NPC muerto, hablar con el comerciante,
// etc.) a llamadas a AudioManager, resolviendo qué archivo(s) usar a través
// de AudioConfig/SpellVfxConfig/ItemVisualConfig/NpcVisualConfig. Ningún
// otro lugar del cliente vuelve a tocar AudioManager directamente ni a
// declarar una ruta de sonido o una lista de variantes a mano.
//
// Todos los métodos son no-op si se construyó con AudioManager == nullptr
// (cliente en modo sin conexión).
class GameAudioService {
public:
    explicit GameAudioService(AudioManager* audio);

    // Combate
    void attack(uint8_t weapon_item, float dist_tiles);
    void spell_cast(uint8_t spell_id, float dist_tiles);
    void npc_death(uint8_t npc_sprite_id, uint16_t entity_id, uint16_t pos_y,
                   float dist_tiles, uint8_t own_weapon_item);
    void player_death(float dist_tiles);

    // Progresión / economía
    void level_up();
    void meditation_start();
    void coins_received();

    // Ambiente (sonido en loop, volumen según distancia)
    void update_ocean_ambient(float dist_tiles);

    // Fauna del bosque (pajaritos, cuervos): suena cada tanto, a intervalos
    // aleatorios, mientras el jugador está dentro de la zona del bosque.
    // Pensado para llamarse todos los frames con la condición actual.
    void update_forest_ambience(bool in_forest);

    // Movimiento
    void footstep_grass();

    // Pasos en piedra de ciudad: audio largo en loop, suena mientras el
    // jugador camina sobre ese piso y se corta apenas se detiene o cambia
    // de piso. Pensado para llamarse todos los frames con la condición.
    void update_city_stone_footsteps(bool walking_on_city_stone);

    // NPC de servicio: comerciante
    void merchant_greet(float dist_tiles);
    void merchant_farewell(float dist_tiles);
    void merchant_list_items(float dist_tiles);
    void merchant_buy(float dist_tiles);

    // NPC de servicio: banquero (no tiene despedida: el original tampoco la tenía)
    void banker_greet(float dist_tiles);
    void banker_deposit(float dist_tiles);
    void banker_withdraw(float dist_tiles);

    // NPC de servicio: sacerdote
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
