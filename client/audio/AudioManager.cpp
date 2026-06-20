#include "AudioManager.h"
#include "../config/AudioConfig.h"
#include <SDL.h>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

AudioManager::AudioManager() {
    const AudioConfig& config = AudioConfig::instance();

    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0)
        throw std::runtime_error(std::string("Mix_Init: ") + Mix_GetError());

    if (Mix_OpenAudio(config.mixer.frequency, MIX_DEFAULT_FORMAT, config.mixer.channels, config.mixer.chunksize) != 0)
        throw std::runtime_error(std::string("Mix_OpenAudio: ") + Mix_GetError());

    Mix_VolumeMusic(config.mixer.default_music_volume);
}

AudioManager::~AudioManager() {
    for (auto& [path, chunk] : _chunk_cache)
        Mix_FreeChunk(chunk);
    if (_music) Mix_FreeMusic(_music);
    Mix_CloseAudio();
    Mix_Quit();
}

void AudioManager::play_music_loop(const std::string& path) {
    Mix_Music* loaded = Mix_LoadMUS(path.c_str());
    if (!loaded) throw std::runtime_error(std::string("Mix_LoadMUS: ") + Mix_GetError());

    if (_music) Mix_FreeMusic(_music);
    _music = loaded;

    Mix_PlayMusic(_music, -1);
}

void AudioManager::set_music_volume(int volume) {
    Mix_VolumeMusic(volume);
}

Mix_Chunk* AudioManager::load_chunk(const std::string& path) {
    auto it = _chunk_cache.find(path);
    if (it != _chunk_cache.end()) return it->second;

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) return nullptr;

    _chunk_cache.emplace(path, chunk);
    return chunk;
}

bool AudioManager::should_throttle(const std::string& path) {
    const AudioConfig& config = AudioConfig::instance();
    uint32_t now = SDL_GetTicks();
    auto it = _last_played_ms.find(path);
    if (it != _last_played_ms.end() && (now - it->second) < config.effect_volumes.effect_cooldown_ms)
        return true;
    _last_played_ms[path] = now;
    return false;
}

void AudioManager::play_effect_now(const std::string& path, float dist_tiles) {
    const AudioConfig& config = AudioConfig::instance();
    if (dist_tiles > config.effect_volumes.max_audible_tiles) return;

    Mix_Chunk* chunk = load_chunk(path);
    if (!chunk) return;

    float t = dist_tiles / config.effect_volumes.max_audible_tiles;  // 0 (cerca) .. 1 (limite audible)
    int volume = static_cast<int>(config.effect_volumes.max_effect_volume - t * 
                 (config.effect_volumes.max_effect_volume - config.effect_volumes.min_effect_volume));

    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel >= 0) Mix_Volume(channel, volume);
}

void AudioManager::play_effect_at(const std::string& path, float dist_tiles) {
    if (should_throttle(path)) return;
    play_effect_now(path, dist_tiles);
}

void AudioManager::play_random_effect_at(const std::vector<std::string>& paths, float dist_tiles) {
    if (paths.empty()) return;
    const std::string& chosen = paths[static_cast<size_t>(rand()) % paths.size()];
    play_effect_at(chosen, dist_tiles);
}

uint32_t AudioManager::chunk_duration_ms(Mix_Chunk* chunk) const {
    if (!chunk) return 600;  // estimacion si el asset no cargo

    int freq = 0, channels = 0;
    Uint16 format = 0;
    if (Mix_QuerySpec(&freq, &format, &channels) == 0 || freq <= 0 || channels <= 0)
        return 600;

    int bytes_per_sample = (SDL_AUDIO_BITSIZE(format) / 8);
    int frame_size = bytes_per_sample * channels;
    if (frame_size <= 0) return 600;

    uint32_t samples = chunk->alen / static_cast<uint32_t>(frame_size);
    return samples * 1000u / static_cast<uint32_t>(freq);
}

void AudioManager::queue_speech_sequence(const std::vector<std::string>& paths, float dist_tiles) {
    static constexpr uint32_t GAP_MS = 200;  // pequeño respiro entre frases

    uint32_t when = SDL_GetTicks();
    for (const auto& path : paths) {
        _speech_queue.push_back({path, dist_tiles, when});
        when += chunk_duration_ms(load_chunk(path)) + GAP_MS;
    }
}

void AudioManager::update() {
    if (_speech_queue.empty()) return;
    uint32_t now = SDL_GetTicks();
    for (auto it = _speech_queue.begin(); it != _speech_queue.end();) {
        if (now >= it->fire_at_ms) {
            play_effect_now(it->path, it->dist_tiles);
            it = _speech_queue.erase(it);
        } else {
            ++it;
        }
    }
}
