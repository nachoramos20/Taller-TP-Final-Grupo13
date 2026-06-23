#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../PlayerState.h"

#include "AssetManager.h"
#include "SpriteLayout.h"
#include "render_types.h"

struct Animation {
    std::vector<SDL2pp::Rect> frames;
};

// Layout de un spritesheet de NPC
struct NpcLayout {
    int frame_w;
    int frame_h;
    int cols;

    static NpcLayout from_sheet(int sheet_w, int sheet_h, int cols, int rows) {
        return {sheet_w / cols, sheet_h / rows, cols};
    }

    SDL2pp::Rect src_rect(int dir_idx, int frame) const {
        return SDL2pp::Rect(frame * frame_w, dir_idx * frame_h, frame_w, frame_h);
    }
};

// Dibuja jugadores y NPCs animados (frame por tick, recorte del
// spritesheet correcto según dirección) y, para jugadores, el equipo
// visible superpuesto en el orden de capas correcto.
class AnimationSystem {
public:
    static constexpr int TICKS_PER_FRAME = 8;
    static constexpr int DIR_SOUTH = 0;
    static constexpr int DIR_NORTH = 1;
    static constexpr int DIR_WEST = 2;
    static constexpr int DIR_EAST = 3;

    AnimationSystem();

    void load();

    // Render de jugador (body + head + equipo opcional).
    // Devuelve los límites reales del sprite dibujado (para overlays).
    // is_ghost atenúa el sprite (transparencia) para distinguir personajes muertos.
    SpriteBounds render(SDL2pp::Renderer& renderer, AssetManager& assets,
                        const std::string& body_path, const std::string& head_path,
                        uint8_t sprite_id, Direction dir, int screen_x, int screen_y, uint32_t tick,
                        bool is_moving, const EquipVisual* equip = nullptr, bool is_ghost = false);

    // Render de NPC (spritesheet único, sin head separado).
    // Devuelve los límites reales del sprite dibujado (para overlays).
    SpriteBounds render_npc(SDL2pp::Renderer& renderer, AssetManager& assets,
                            const std::string& sheet_path, int cols, int rows, int frame_w,
                            int frame_h, Direction dir, int screen_x, int screen_y, uint32_t tick,
                            bool is_moving, float scale = 1.5f, int draw_offset_y = 0);

private:
    int frame_for_tick(uint32_t tick, int n_frames) const;
    int direction_to_index(Direction dir) const;

    void render_weapon(SDL2pp::Renderer& renderer, AssetManager& assets,
                       const std::string& weapon_path, int dir_idx, int frame,
                       const SDL2pp::Rect& body_dst, float scale);

    Animation _body_anims[4];
    int _last_sprite_id;
    SDL2pp::Rect _head_rects[4];
    int _head_overlaps[4];
    int _head_offset_x[4];
    float _armor_scale_y = 1.0f;
};
