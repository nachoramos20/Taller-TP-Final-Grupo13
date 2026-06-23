#include "AssetManager.h"

#include <stdexcept>

#include "SDL_image.h"

AssetManager::AssetManager(SDL2pp::Renderer& renderer): _renderer(renderer) {}

SDL2pp::Texture& AssetManager::get(const std::string& path) {
    auto it = _cache.find(path);
    if (it != _cache.end())
        return it->second;

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface)
        throw std::runtime_error(std::string("No se pudo cargar: ") + path);

    SDL_Texture* raw = SDL_CreateTextureFromSurface(_renderer.Get(), surface);
    SDL_FreeSurface(surface);

    if (!raw)
        throw std::runtime_error(std::string("No se pudo crear textura: ") + path);

    _cache.emplace(path, SDL2pp::Texture(raw));
    return _cache.at(path);
}
