#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../render/AssetManager.h"
#include "../render/AnimationSystem.h"
#include "../render/SpriteConfig.h"
#include "../render/ObjectSupConfig.h"
#include "../render/Camera.h"
#include "../../common/protocol/protocol.h"
#include "WorldState.h"
#include "PlayerState.h"

class InventoryPanel;

// Dibuja todo lo que compone el mundo de juego: piso, objetos superiores,
// entidades (jugadores/NPCs/ítems en el piso), barras de vida y nombres,
// hechizos, proyectiles y efectos de muerte. Es dueño de los recursos de
// rendering (AssetManager/AnimationSystem) y de los config de sprites
// (SpriteConfig/TileConfig/ObjectSupConfig); resuelve los datos visuales de
// ítems y NPCs vía ItemVisualConfig/NpcVisualConfig en lugar de tenerlos
// hardcodeados.
class WorldRenderer {
public:
    WorldRenderer(SDL2pp::Window& window, SDL2pp::Renderer& renderer, Camera& camera);

    // Registra una sola vez los íconos de inventario conocidos.
    void load_item_textures(InventoryPanel& inventory);

    void render(WorldState& state, const PlayerState& player);

private:
    void render_floor(const WorldState& state);
    void render_obj_sup(const WorldState& state);
    void render_entities(WorldState& state, const PlayerState& player);
    void render_spells(WorldState& state);
    void render_projectiles(WorldState& state);
    void render_deaths(WorldState& state);

    void render_floor_item(const EntityDTO& e, int screen_x, int screen_y);
    void render_npc(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
                     Direction dir, bool moving);
    void render_player_like(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
                             Direction dir, bool moving);
    void render_entity_healthbar(const EntityDTO& entity, const SpriteBounds& bounds,
                                  bool is_service_npc, int name_label_offset_y);

    EquipVisual build_equip_visual(uint8_t item_weapon, uint8_t item_armor, uint8_t item_helmet,
                                    uint8_t item_shield, uint8_t race_sprite_id) const;

    SDL2pp::Window&   _window;
    SDL2pp::Renderer& _renderer;
    Camera&           _camera;

    AssetManager     _assets;
    AnimationSystem  _anim;
    SpriteConfig     _sprite_config;
    TileConfig       _tile_config;
    ObjectSupConfig  _obj_sup_config;
    TTF_Font*        _small_font = nullptr;  // nombres y barras de vida
};
