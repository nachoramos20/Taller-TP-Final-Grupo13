#include "GameAudioService.h"
#include "AudioManager.h"
#include "../config/AudioConfig.h"
#include "../config/SpellVfxConfig.h"
#include "../render/ItemVisualConfig.h"
#include "../render/NpcVisualConfig.h"

GameAudioService::GameAudioService(AudioManager* audio) : _audio(audio) {}

void GameAudioService::play_random(const std::vector<std::string>& paths, float dist_tiles) {
    if (!_audio || paths.empty()) return;
    _audio->play_random_effect_at(paths, dist_tiles);
}

void GameAudioService::speak_sequence(const std::vector<std::string>& lines, float dist_tiles, uint32_t gap_ms) {
    if (!_audio || lines.empty()) return;
    _audio->speak(lines, dist_tiles, gap_ms);
}

void GameAudioService::speak_random(const std::vector<std::string>& lines, float dist_tiles, uint32_t gap_ms) {
    if (!_audio || lines.empty()) return;
    _audio->speak_random(lines, dist_tiles, gap_ms);
}

void GameAudioService::attack(uint8_t weapon_item, float dist_tiles) {
    if (!_audio) return;
    const auto& cfg = AudioConfig::instance();

    if (weapon_item != 0) {
        const auto& item = ItemVisualConfig::instance().get(weapon_item);
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

void GameAudioService::spell_cast(uint8_t spell_id, float dist_tiles) {
    if (!_audio || spell_id == 0) return;
    const std::string& sound_path = SpellVfxConfig::instance().get_effect_info(spell_id).sound_path;
    if (sound_path.empty()) return;
    _audio->play_effect_at(sound_path, dist_tiles);
}

void GameAudioService::npc_death(uint8_t npc_sprite_id, uint16_t entity_id, uint16_t pos_y,
                                  float dist_tiles, uint8_t own_weapon_item) {
    if (!_audio) return;

    const auto& variant = NpcVisualConfig::instance().select_variant(npc_sprite_id, entity_id, pos_y);
    if (!variant.death_sound_key.empty())
        play_random(AudioConfig::instance().get_creature_sound(variant.death_sound_key), dist_tiles);

    // Golpe final con sangre: sólo si el observador tiene esa arma equipada.
    const auto& weapon = ItemVisualConfig::instance().get(own_weapon_item);
    if (!weapon.melee_finisher_sound_key.empty())
        play_random(AudioConfig::instance().get_combat_melee_sound(weapon.melee_finisher_sound_key), dist_tiles);
}

void GameAudioService::player_death(float dist_tiles) {
    play_random(AudioConfig::instance().get_death_sound("player_death"), dist_tiles);
}

void GameAudioService::level_up() {
    play_random(AudioConfig::instance().get_ui_sound("level_up"), 0.0f);
}

void GameAudioService::meditation_start() {
    play_random(AudioConfig::instance().get_ambient_sound("meditation"), 0.0f);
}

void GameAudioService::coins_received() {
    play_random(AudioConfig::instance().get_economy_sound("monedas"), 0.0f);
}

void GameAudioService::update_ocean_ambient(float dist_tiles) {
    if (!_audio) return;
    const auto& ocean = AudioConfig::instance().get_ambient_sound("ocean");
    if (ocean.empty()) return;
    _audio->set_ambient_loop(ocean.front(), dist_tiles);
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
