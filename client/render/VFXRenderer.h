#pragma once

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../WorldState.h"

#include "AssetManager.h"
#include "Camera.h"

// Dibuja efectos visuales transitorios (hechizos, proyectiles, muertes),
// cada uno con el mismo patrón de "expirar los vencidos, dibujar el resto
// según tiempo transcurrido". Extraída de WorldRenderer: a diferencia del
// piso/objetos/entidades, no necesita SpriteConfig/AnimationSystem ni
// conocer EntityDTO, solo el Camera/AssetManager compartidos.
class VFXRenderer {
public:
    VFXRenderer(SDL2pp::Renderer& renderer, Camera& camera, AssetManager& assets);

    void render_spells(WorldState& state);
    void render_projectiles(WorldState& state);
    void render_deaths(WorldState& state);

private:
    SDL2pp::Renderer& _renderer;
    Camera& _camera;
    AssetManager& _assets;
};
