#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "AssetManager.h"
#include "AnimationSystem.h"
#include "render_types.h"
#include "SpriteConfig.h"
#include "Camera.h"
#include "../../common/protocol/protocol.h"
#include "../WorldState.h"
#include "../PlayerState.h"

// Dibuja entidades (jugadores/NPCs/ítems en el piso) con su barra de vida
// y nombre, resolviendo equipo visible vía ItemVisualConfig/NpcVisualConfig.
// Extraída de WorldRenderer (que además dibuja piso/objetos superiores y
// VFX transitorios — ver VFXRenderer): responsabilidad propia, con su
// propio AnimationSystem/SpriteConfig que no usa el resto de WorldRenderer.
class EntityRenderer {
public:
    EntityRenderer(SDL2pp::Renderer& renderer, Camera& camera, AssetManager& assets);

    void render(WorldState& state, const PlayerState& player);

private:
    void render_floor_item(const EntityDTO& e, int screen_x, int screen_y);
    void render_npc(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
                     Direction dir, bool moving);
    void render_player_like(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
                             Direction dir, bool moving);
    void render_entity_healthbar(const EntityDTO& entity, const SpriteBounds& bounds,
                                  bool is_service_npc, int name_label_offset_y);

    EquipVisual build_equip_visual(uint8_t item_weapon, uint8_t item_armor, uint8_t item_helmet,
                                    uint8_t item_shield, uint8_t race_sprite_id) const;

    SDL2pp::Renderer& _renderer;
    Camera&           _camera;
    AssetManager&     _assets;

    AnimationSystem _anim;
    SpriteConfig    _sprite_config;
    TTF_Font*       _small_font = nullptr;  // nombres y barras de vida
};
