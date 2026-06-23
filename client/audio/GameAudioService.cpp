#include "GameAudioService.h"

#include <cstdlib>

#include <SDL.h>

#include "../config/AudioConfig.h"
#include "../config/ItemVisualConfig.h"
#include "../config/NpcVisualConfig.h"
#include "../config/SpellVfxConfig.h"

#include "AudioManager.h"

namespace {
constexpr float FOOTSTEP_GRASS_VOLUME_SCALE = 0.2f;
constexpr float FOOTSTEP_CITY_STONE_VOLUME_SCALE = 0.3f;
constexpr float DAMAGE_RECEIVED_VOLUME_SCALE =
        0.35f;  // apuñalada al ser atacado: más bajo que el propio ataque
constexpr float FOREST_AMBIENCE_VOLUME_SCALE = 0.2f;
constexpr uint32_t FOREST_SOUND_MIN_INTERVAL_MS = 8000;
constexpr uint32_t FOREST_SOUND_MAX_INTERVAL_MS = 20000;
constexpr float CEMETERY_WIND_MAX_AUDIBLE_TILES = 8.0f;
}  // namespace

GameAudioService::GameAudioService(AudioManager* audio): _audio(audio) {}

void GameAudioService::play_random(const std::vector<std::string>& paths, float dist_tiles,
                                   float volume_scale) {
    if (!_audio || paths.empty())
        return;
    _audio->play_random_effect_at(paths, dist_tiles, volume_scale);
}

void GameAudioService::play_sequential(const std::vector<std::string>& paths, float dist_tiles,
                                       size_t& index, float volume_scale) {
    if (!_audio || paths.empty())
        return;
    _audio->play_effect_at(paths[index % paths.size()], dist_tiles, volume_scale);
    index++;
}

void GameAudioService::speak_sequence(const std::vector<std::string>& lines, float dist_tiles,
                                      uint32_t gap_ms) {
    if (!_audio || lines.empty())
        return;
    _audio->speak(lines, dist_tiles, gap_ms);
}

void GameAudioService::speak_random(const std::vector<std::string>& lines, float dist_tiles,
                                    uint32_t gap_ms) {
    if (!_audio || lines.empty())
        return;
    _audio->speak_random(lines, dist_tiles, gap_ms);
}

void GameAudioService::attack(uint8_t weapon_item, float dist_tiles) {
    if (!_audio)
        return;
    const AudioConfig& cfg = AudioConfig::instance();

    if (weapon_item != 0) {
        const ItemVisualEntry& item = ItemVisualConfig::instance().get(weapon_item);
        if (item.attack_sound_category == "melee") {
            play_random(cfg.get_combat_melee_sound(item.attack_sound_key), dist_tiles);
            return;
        }
        if (item.attack_sound_category == "ranged") {
            play_random(cfg.get_combat_ranged_sound(item.attack_sound_key), dist_tiles);
            return;
        }
        if (item.attack_sound_category == "magic") {
            play_random(cfg.get_magic_sound(item.attack_sound_key), dist_tiles);
            return;
        }
    }
    play_random(cfg.get_combat_melee_sound("generico"), dist_tiles);
}

void GameAudioService::damage_received(float dist_tiles) {
    if (!_audio)
        return;
    play_random(AudioConfig::instance().get_combat_melee_sound("generico"), dist_tiles,
                DAMAGE_RECEIVED_VOLUME_SCALE);
}

void GameAudioService::spell_cast(uint8_t spell_id, float dist_tiles) {
    if (!_audio || spell_id == 0)
        return;
    const std::string& sound_path = SpellVfxConfig::instance().get_effect_info(spell_id).sound_path;
    if (sound_path.empty())
        return;
    _audio->play_effect_at(sound_path, dist_tiles);
}

void GameAudioService::npc_death(uint8_t npc_sprite_id, uint16_t entity_id, uint16_t pos_y,
                                 float dist_tiles, uint8_t own_weapon_item) {
    if (!_audio)
        return;

    const NpcSheetVariant& variant =
            NpcVisualConfig::instance().select_variant(npc_sprite_id, entity_id, pos_y);
    if (!variant.death_sound_key.empty())
        play_random(AudioConfig::instance().get_creature_sound(variant.death_sound_key),
                    dist_tiles);

    // Golpe final con sangre: sólo si el observador tiene esa arma equipada.
    const ItemVisualEntry& weapon = ItemVisualConfig::instance().get(own_weapon_item);
    if (!weapon.melee_finisher_sound_key.empty())
        play_random(AudioConfig::instance().get_combat_melee_sound(weapon.melee_finisher_sound_key),
                    dist_tiles);
}

void GameAudioService::player_death(float dist_tiles) {
    play_random(AudioConfig::instance().get_death_sound("player_death"), dist_tiles);
}

void GameAudioService::level_up() {
    play_random(AudioConfig::instance().get_ui_sound("level_up"), 0.0f);
}

void GameAudioService::update_meditation_loop(bool meditating) {
    if (!_audio)
        return;
    const std::vector<std::string>& sound = AudioConfig::instance().get_ambient_sound("meditation");
    if (sound.empty())
        return;
    _audio->set_meditation_loop(sound.front(), meditating);
}

void GameAudioService::coins_received() {
    play_random(AudioConfig::instance().get_economy_sound("monedas"), 0.0f);
}

void GameAudioService::potion_used() {
    play_random(AudioConfig::instance().get_economy_sound("potion"), 0.0f);
}

void GameAudioService::player_spawn() {
    play_random(AudioConfig::instance().get_ui_sound("player_spawn"), 0.0f);
}

void GameAudioService::click() { play_random(AudioConfig::instance().get_ui_sound("click"), 0.0f); }

void GameAudioService::clan_created() {
    play_random(AudioConfig::instance().get_ui_sound("clan_created"), 0.0f);
}

void GameAudioService::clan_member_attacked() {
    play_random(AudioConfig::instance().get_ui_sound("clan_alert"), 0.0f);
}

void GameAudioService::private_message_received() {
    play_random(AudioConfig::instance().get_ui_sound("private_message"), 0.0f);
}

void GameAudioService::update_ocean_ambient(float dist_tiles) {
    if (!_audio)
        return;
    const std::vector<std::string>& ocean = AudioConfig::instance().get_ambient_sound("ocean");
    if (ocean.empty())
        return;
    _audio->set_ambient_loop(ocean.front(), dist_tiles);
}

void GameAudioService::update_cemetery_ambient(float dist_tiles) {
    if (!_audio)
        return;
    const std::vector<std::string>& wind =
            AudioConfig::instance().get_ambient_sound("cemetery_wind");
    if (wind.empty())
        return;
    _audio->set_secondary_ambient_loop(wind.front(), dist_tiles, CEMETERY_WIND_MAX_AUDIBLE_TILES);
}

void GameAudioService::update_forest_ambience(bool in_forest) {
    if (!_audio || !in_forest)
        return;

    uint32_t now = SDL_GetTicks();
    if (now < _next_forest_sound_ms)
        return;

    play_random(AudioConfig::instance().get_ambient_sound("bosque_fauna"), 0.0f,
                FOREST_AMBIENCE_VOLUME_SCALE);

    uint32_t span = FOREST_SOUND_MAX_INTERVAL_MS - FOREST_SOUND_MIN_INTERVAL_MS;
    _next_forest_sound_ms =
            now + FOREST_SOUND_MIN_INTERVAL_MS + static_cast<uint32_t>(rand()) % span;
}

void GameAudioService::footstep_grass() {
    play_sequential(AudioConfig::instance().get_movement_sound("paso_pasto"), 0.0f,
                    _footstep_grass_index, FOOTSTEP_GRASS_VOLUME_SCALE);
}

void GameAudioService::update_city_stone_footsteps(bool walking_on_city_stone) {
    if (!_audio)
        return;
    const std::vector<std::string>& paths =
            AudioConfig::instance().get_movement_sound("pasos_en_grava");
    if (paths.empty())
        return;
    _audio->set_looping_while(paths.front(), walking_on_city_stone,
                              FOOTSTEP_CITY_STONE_VOLUME_SCALE);
}

void GameAudioService::merchant_greet(float dist_tiles) {
    speak_sequence(AudioConfig::instance().get_npc_merchant_sound("saludo"), dist_tiles);
}

void GameAudioService::merchant_farewell(float dist_tiles) {
    speak_random(AudioConfig::instance().get_npc_merchant_sound("despedida"), dist_tiles);
}

void GameAudioService::merchant_list_items(float dist_tiles) {
    speak_random(AudioConfig::instance().get_npc_merchant_sound("lo_pides_lo_tienes"), dist_tiles);
}

void GameAudioService::merchant_buy(float dist_tiles) {
    speak_random(AudioConfig::instance().get_npc_merchant_sound("una_gran_eleccion"), dist_tiles);
}

void GameAudioService::banker_greet(float dist_tiles) {
    speak_sequence(AudioConfig::instance().get_npc_banker_sound("saludo"), dist_tiles, 10);
}

void GameAudioService::banker_deposit(float dist_tiles) {
    speak_random(AudioConfig::instance().get_npc_banker_sound("cuidamos_sus_cosas"), dist_tiles);
}

void GameAudioService::banker_withdraw(float dist_tiles) {
    speak_random(AudioConfig::instance().get_npc_banker_sound("puede_confiar"), dist_tiles);
}

void GameAudioService::priest_greet(float dist_tiles) {
    speak_random(AudioConfig::instance().get_npc_priest_sound("saludo"), dist_tiles);
}

void GameAudioService::priest_farewell(float dist_tiles) {
    speak_sequence(AudioConfig::instance().get_npc_priest_sound("despedida"), dist_tiles);
}

void GameAudioService::priest_heal(float dist_tiles) {
    speak_sequence(AudioConfig::instance().get_npc_priest_sound("curar"), dist_tiles);
}

void GameAudioService::priest_resurrect(float dist_tiles) {
    speak_random(AudioConfig::instance().get_npc_priest_sound("resucitar"), dist_tiles);
}
