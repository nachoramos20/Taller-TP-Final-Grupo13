#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <string>
#include <vector>
#include "AssetManager.h"
#include "SpriteLayout.h"
#include "../game/PlayerState.h"

struct Animation {
    std::vector<SDL2pp::Rect> frames;
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

    void render(SDL2pp::Renderer& renderer,
                AssetManager& assets,
                const std::string& body_path,
                const std::string& head_path,
                uint8_t sprite_id,
                Direction dir,
                int screen_x, int screen_y,
                uint32_t tick,
                bool is_moving);

private:
    int frame_for_tick(uint32_t tick, int dir_idx) const;
    int direction_to_index(Direction dir) const;

    Animation    _body_anims[4];
    int          _last_sprite_id;
    SDL2pp::Rect _head_rects[4];
    int          _head_overlaps[4];
};