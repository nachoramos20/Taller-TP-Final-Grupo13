#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <string>
#include <unordered_map>

// Cachea texturas cargadas desde disco por path, para no recargar el
// mismo archivo cada vez que algo necesita dibujarlo.
class AssetManager {
public:
    explicit AssetManager(SDL2pp::Renderer& renderer);
    SDL2pp::Texture& get(const std::string& path);

private:
    SDL2pp::Renderer& _renderer;
    std::unordered_map<std::string, SDL2pp::Texture> _cache;
};