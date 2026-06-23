#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2pp/SDL2pp.hh>

#include "../../common/protocol/protocol.h"
#include "../PlayerState.h"
#include "../WorldState.h"

#include "AssetManager.h"
#include "Camera.h"
#include "EntityRenderer.h"
#include "ObjectSupConfig.h"
#include "SpriteConfig.h"
#include "VFXRenderer.h"

class InventoryPanel;

// Dibuja piso y objetos superiores, y orquesta a EntityRenderer (entidades,
// barras de vida, nombres) y VFXRenderer (hechizos, proyectiles, efectos
// de muerte) en el orden de capas correcto. Es dueño de los recursos de
// rendering compartidos (AssetManager) y de los config de piso/objetos
// (TileConfig/ObjectSupConfig); SpriteConfig/AnimationSystem ahora viven
// en EntityRenderer, que es lo único que los necesita.
class WorldRenderer {
public:
    WorldRenderer(SDL2pp::Window& window, SDL2pp::Renderer& renderer, Camera& camera);

    // Registra una sola vez los íconos de inventario conocidos.
    void load_item_textures(InventoryPanel& inventory);

    void render(WorldState& state, const PlayerState& player);

private:
    void render_floor(const WorldState& state);
    void render_obj_sup(const WorldState& state);

    SDL2pp::Window& _window;
    SDL2pp::Renderer& _renderer;
    Camera& _camera;

    AssetManager _assets;
    TileConfig _tile_config;
    ObjectSupConfig _obj_sup_config;

    EntityRenderer _entity_renderer;
    VFXRenderer _vfx_renderer;
};
