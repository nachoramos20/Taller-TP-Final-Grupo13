#pragma once

#include <SDL2pp/SDL2pp.hh>
#include <string>
#include <vector>
#include "AssetManager.h"
#include "../game/PlayerState.h"

struct Animation {
    std::vector<SDL2pp::Rect> frames;
};

class AnimationSystem {
public:
    static constexpr int BODY_W       = 27;
    static constexpr int BODY_H       = 48;
    static constexpr int HEAD_W       = 27;
    static constexpr int HEAD_H       = 17;
    static constexpr int HEAD_OVERLAP = 4;
    static constexpr int TICKS_PER_FRAME = 8;

    AnimationSystem();

    void load();

    void render(SDL2pp::Renderer& renderer,
                AssetManager& assets,
                const std::string& body_path,
                const std::string& head_path,
                Direction dir,
                int screen_x, int screen_y,
                uint32_t tick,
                bool is_moving);

private:
    int frame_for_tick(uint32_t tick, int dir_idx) const;
    int direction_to_index(Direction dir) const;

    Animation        _body_anims[4];
    SDL2pp::Rect     _head_rects[4];
    int              _frame_counts[4];
};