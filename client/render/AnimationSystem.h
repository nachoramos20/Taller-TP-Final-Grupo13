#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <string>
#include <vector>
#include <unordered_map>
#include "AssetManager.h"
#include "SpriteLayout.h"
#include "../game/PlayerState.h"

struct Animation {
    std::vector<SDL2pp::Rect> frames;
};

// Layout de un spritesheet de NPC
struct NpcLayout {
    int frame_w;
    int frame_h;
    int cols;

    static NpcLayout from_sheet(int sheet_w, int sheet_h, int cols, int rows) {
        return { sheet_w / cols, sheet_h / rows, cols };
    }

    SDL2pp::Rect src_rect(int dir_idx, int frame) const {
        return SDL2pp::Rect(frame * frame_w, dir_idx * frame_h, frame_w, frame_h);
    }
};

// Límites reales del sprite ya dibujado en pantalla
struct SpriteBounds {
    int top_y;
    int center_x;
    int width;
};

// Equipo visual a renderizar encima del personaje
struct EquipVisual {
    std::string armor_path;
    std::string helmet_path;
    int         helmet_src_x = 0;
    int         helmet_src_y = 0;
    int         helmet_src_w = 0;
    int         helmet_src_h = 0;
    std::string weapon_path;
    std::string shield_path;
};

class AnimationSystem {
public:
    static constexpr int TICKS_PER_FRAME = 8;
    static constexpr int DIR_SOUTH = 0;
    static constexpr int DIR_NORTH = 1;
    static constexpr int DIR_WEST  = 2;
    static constexpr int DIR_EAST  = 3;

    AnimationSystem();

    void load();

    // Render de jugador (body + head + equipo opcional).
    // Devuelve los límites reales del sprite dibujado (para overlays).
    SpriteBounds render(SDL2pp::Renderer& renderer,
                AssetManager& assets,
                const std::string& body_path,
                const std::string& head_path,
                uint8_t sprite_id,
                Direction dir,
                int screen_x, int screen_y,
                uint32_t tick,
                bool is_moving,
                const EquipVisual* equip = nullptr);

    // Render de NPC (spritesheet único, sin head separado).
    // Devuelve los límites reales del sprite dibujado (para overlays).
    SpriteBounds render_npc(SDL2pp::Renderer& renderer,
                    AssetManager& assets,
                    const std::string& sheet_path,
                    int cols, int rows,
                    int frame_w, int frame_h,
                    Direction dir,
                    int screen_x, int screen_y,
                    uint32_t tick,
                    bool is_moving);

private:
    int frame_for_tick(uint32_t tick, int n_frames) const;
    int direction_to_index(Direction dir) const;

    void render_weapon(SDL2pp::Renderer& renderer,
                       AssetManager& assets,
                       const std::string& weapon_path,
                       int dir_idx, int frame,
                       const SDL2pp::Rect& body_dst);

    Animation    _body_anims[4];
    int          _last_sprite_id;
    SDL2pp::Rect _head_rects[4];
    int          _head_overlaps[4];
};
