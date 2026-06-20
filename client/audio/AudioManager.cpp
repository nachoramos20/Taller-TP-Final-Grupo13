#include "AudioManager.h"
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

static constexpr int MIXER_FREQUENCY = MIX_DEFAULT_FREQUENCY;
static constexpr int MIXER_CHANNELS  = 2;
static constexpr int MIXER_CHUNKSIZE = 1024;
static constexpr int DEFAULT_MUSIC_VOLUME = 20;  // 0-128 (MIX_MAX_VOLUME);

static constexpr float MAX_AUDIBLE_TILES   = 18.0f;  
static constexpr int   MIN_EFFECT_VOLUME   = 0;
static constexpr int   MAX_EFFECT_VOLUME   = 110;    
static constexpr uint32_t EFFECT_COOLDOWN_MS = 80;   

static constexpr int TOTAL_CHANNELS  = 16;
static constexpr int SPEECH_CHANNEL  = 0;  
static constexpr int AMBIENT_CHANNEL = 1; 

AudioManager::AudioManager() {
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0)
        throw std::runtime_error(std::string("Mix_Init: ") + Mix_GetError());

    if (Mix_OpenAudio(MIXER_FREQUENCY, MIX_DEFAULT_FORMAT, MIXER_CHANNELS, MIXER_CHUNKSIZE) != 0)
        throw std::runtime_error(std::string("Mix_OpenAudio: ") + Mix_GetError());

    Mix_AllocateChannels(TOTAL_CHANNELS);
    Mix_ReserveChannels(2);

    Mix_VolumeMusic(DEFAULT_MUSIC_VOLUME);
}

AudioManager::~AudioManager() {
    for (auto& [path, chunk] : _chunk_cache)
        Mix_FreeChunk(chunk);
    for (auto& [path, chunk] : _speech_chunk_cache)
        Mix_FreeChunk(chunk);
    for (auto& [path, chunk] : _ambient_chunk_cache)
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

// Recorta ese silencio analizando la energía del audio. lead_pad_ms/
// tail_pad_ms dejan un margen para no comerse el ataque/decaimiento del
// sonido: para voz conviene generoso (no cortar consonantes), para un loop
// ambiente conviene ajustado (que el empalme se note lo menos posible).
Mix_Chunk* AudioManager::trim_silence(Mix_Chunk* chunk, uint32_t lead_pad_ms, uint32_t tail_pad_ms) const {
    if (!chunk || chunk->alen < 4) return chunk;

    int freq = 0, channels = 0;
    Uint16 format = 0;
    if (Mix_QuerySpec(&freq, &format, &channels) == 0 || freq <= 0 || channels <= 0)
        return chunk;
    if (format != AUDIO_S16LSB && format != AUDIO_S16MSB) return chunk;  // solo soporta PCM de 16 bits

    static constexpr int16_t SILENCE_THRESHOLD = 50;   // umbral de amplitud (escala 0-32767)
    static constexpr uint32_t WINDOW_MS = 20;

    const auto* samples = reinterpret_cast<const int16_t*>(chunk->abuf);
    uint32_t total_samples = chunk->alen / sizeof(int16_t);
    uint32_t total_frames  = total_samples / static_cast<uint32_t>(channels);
    uint32_t window_frames = std::max<uint32_t>(1, static_cast<uint32_t>(freq) * WINDOW_MS / 1000);

    uint32_t first_loud = UINT32_MAX, last_loud = 0;
    for (uint32_t f = 0; f < total_frames; f += window_frames) {
        uint32_t end_f = std::min(total_frames, f + window_frames);
        int64_t sum_sq = 0;
        uint32_t count = 0;
        for (uint32_t s = f * channels; s < end_f * channels; ++s) {
            sum_sq += static_cast<int64_t>(samples[s]) * samples[s];
            ++count;
        }
        if (count == 0) continue;
        double rms = std::sqrt(static_cast<double>(sum_sq) / count);
        if (rms > SILENCE_THRESHOLD) {
            if (first_loud == UINT32_MAX) first_loud = f;
            last_loud = end_f;
        }
    }
    if (first_loud == UINT32_MAX) return chunk;  // todo silencio: no tocar

    uint32_t lead_pad   = static_cast<uint32_t>(freq) * lead_pad_ms / 1000;
    uint32_t tail_pad   = static_cast<uint32_t>(freq) * tail_pad_ms / 1000;
    uint32_t start_frame = (first_loud > lead_pad) ? first_loud - lead_pad : 0;
    uint32_t end_frame   = std::min(total_frames, last_loud + tail_pad);
    if (end_frame <= start_frame || (start_frame == 0 && end_frame == total_frames))
        return chunk;  // nada para recortar

    uint32_t frame_size = static_cast<uint32_t>(channels) * sizeof(int16_t);
    uint32_t new_len = (end_frame - start_frame) * frame_size;

    Uint8* new_buf = static_cast<Uint8*>(SDL_malloc(new_len));
    if (!new_buf) return chunk;
    std::memcpy(new_buf, chunk->abuf + start_frame * frame_size, new_len);

    Mix_Chunk* trimmed = static_cast<Mix_Chunk*>(SDL_malloc(sizeof(Mix_Chunk)));
    if (!trimmed) { SDL_free(new_buf); return chunk; }
    trimmed->allocated = 1;
    trimmed->abuf       = new_buf;
    trimmed->alen       = new_len;
    trimmed->volume     = chunk->volume;

    Mix_FreeChunk(chunk);
    return trimmed;
}

Mix_Chunk* AudioManager::load_speech_chunk(const std::string& path) {
    auto it = _speech_chunk_cache.find(path);
    if (it != _speech_chunk_cache.end()) return it->second;

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) return nullptr;

    chunk = trim_silence(chunk, 30, 80);

    _speech_chunk_cache.emplace(path, chunk);
    return chunk;
}

Mix_Chunk* AudioManager::load_ambient_chunk(const std::string& path) {
    auto it = _ambient_chunk_cache.find(path);
    if (it != _ambient_chunk_cache.end()) return it->second;

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) return nullptr;

    chunk = trim_silence(chunk, 5, 5);  // margen mínimo: que el loop empalme casi sin corte

    _ambient_chunk_cache.emplace(path, chunk);
    return chunk;
}

bool AudioManager::should_throttle(const std::string& path) {
    uint32_t now = SDL_GetTicks();
    auto it = _last_played_ms.find(path);
    if (it != _last_played_ms.end() && (now - it->second) < EFFECT_COOLDOWN_MS)
        return true;
    _last_played_ms[path] = now;
    return false;
}

void AudioManager::play_effect_now(const std::string& path, float dist_tiles) {
    if (dist_tiles > MAX_AUDIBLE_TILES) return;

    Mix_Chunk* chunk = load_chunk(path);
    if (!chunk) return;

    float t = dist_tiles / MAX_AUDIBLE_TILES;  // 0 (cerca) .. 1 (limite audible)
    int volume = static_cast<int>(MAX_EFFECT_VOLUME - t * (MAX_EFFECT_VOLUME - MIN_EFFECT_VOLUME));

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

void AudioManager::play_speech_now(const std::string& path, float dist_tiles) {
    if (dist_tiles > MAX_AUDIBLE_TILES) return;

    Mix_Chunk* chunk = load_speech_chunk(path);
    if (!chunk) return;

    float t = dist_tiles / MAX_AUDIBLE_TILES;
    int volume = static_cast<int>(MAX_EFFECT_VOLUME - t * (MAX_EFFECT_VOLUME - MIN_EFFECT_VOLUME));

    Mix_PlayChannel(SPEECH_CHANNEL, chunk, 0);
    Mix_Volume(SPEECH_CHANNEL, volume);
}

void AudioManager::speak(const std::vector<std::string>& paths, float dist_tiles, uint32_t gap_ms) {
    if (paths.empty()) return;

    // Corta lo que esté sonando o pendiente y arranca esta secuencia ya.
    _speech_queue.clear();
    Mix_HaltChannel(SPEECH_CHANNEL);

    play_speech_now(paths[0], dist_tiles);

    uint32_t when = SDL_GetTicks() + chunk_duration_ms(load_speech_chunk(paths[0])) + gap_ms;
    for (size_t i = 1; i < paths.size(); ++i) {
        _speech_queue.push_back({paths[i], dist_tiles, when});
        when += chunk_duration_ms(load_speech_chunk(paths[i])) + gap_ms;
    }
}

void AudioManager::speak_random(const std::vector<std::string>& paths, float dist_tiles, uint32_t gap_ms) {
    if (paths.empty()) return;
    const std::string& chosen = paths[static_cast<size_t>(rand()) % paths.size()];
    speak({chosen}, dist_tiles, gap_ms);
}

void AudioManager::set_ambient_loop(const std::string& path, float dist_tiles) {
    if (dist_tiles > MAX_AUDIBLE_TILES) {
        if (!_ambient_path.empty()) {
            Mix_HaltChannel(AMBIENT_CHANNEL);
            _ambient_path.clear();
        }
        return;
    }

    if (_ambient_path != path) {
        Mix_Chunk* chunk = load_ambient_chunk(path);
        if (!chunk) return;
        Mix_HaltChannel(AMBIENT_CHANNEL);
        Mix_PlayChannel(AMBIENT_CHANNEL, chunk, -1);  // loop infinito
        _ambient_path = path;
    }

    float t = dist_tiles / MAX_AUDIBLE_TILES;  // 0 (cerca) .. 1 (limite audible)
    int volume = static_cast<int>(MAX_EFFECT_VOLUME - t * (MAX_EFFECT_VOLUME - MIN_EFFECT_VOLUME));
    Mix_Volume(AMBIENT_CHANNEL, volume);
}

void AudioManager::update() {
    if (_speech_queue.empty()) return;
    uint32_t now = SDL_GetTicks();
    if (now >= _speech_queue.front().fire_at_ms) {
        play_speech_now(_speech_queue.front().path, _speech_queue.front().dist_tiles);
        _speech_queue.erase(_speech_queue.begin());
    }
}
