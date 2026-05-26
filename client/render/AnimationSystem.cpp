#include "AnimationSystem.h"

static constexpr int DIR_SOUTH = 0;
static constexpr int DIR_NORTH = 1;
static constexpr int DIR_EAST  = 3;
static constexpr int DIR_WEST  = 2;

AnimationSystem::AnimationSystem() {
    _frame_counts[0] = 6;  // sur
    _frame_counts[1] = 6;  // norte
    _frame_counts[2] = 5;  // este
    _frame_counts[3] = 5;  // oeste
}

void AnimationSystem::load() {
    int row_y[4]  = {0, 48, 96, 144};

    // Cuerpo
    for (int dir = 0; dir < 4; dir++) {
        _body_anims[dir].frames.clear();
        for (int f = 0; f < _frame_counts[dir]; f++) {
            _body_anims[dir].frames.push_back(
                SDL2pp::Rect(f * BODY_W, row_y[dir], BODY_W, BODY_H));
        }
    }

    // Cabeza — primera cabeza de cada fila del 422.png
    // Sur=0, Norte=1, Este=2, Oeste=3
    int head_y[4] = {9,  78,  142, 206};
    int head_h[4] = {21, 16,  15,  18};

    for (int dir = 0; dir < 4; dir++) {
        _head_rects[dir] = SDL2pp::Rect(0, head_y[dir], HEAD_W, head_h[dir]);
    }
}

int AnimationSystem::frame_for_tick(uint32_t tick, int dir_idx) const {
    return (tick / TICKS_PER_FRAME) % _frame_counts[dir_idx];
}

int AnimationSystem::direction_to_index(Direction dir) const {
    switch (dir) {
        case Direction::SOUTH: return DIR_SOUTH;
        case Direction::NORTH: return DIR_NORTH;
        case Direction::EAST:  return DIR_EAST;
        case Direction::WEST:  return DIR_WEST;
        default:               return DIR_SOUTH;
    }
}

void AnimationSystem::render(SDL2pp::Renderer& renderer,
                             AssetManager& assets,
                             const std::string& body_path,
                             const std::string& head_path,
                             Direction dir,
                             int screen_x, int screen_y,
                             uint32_t tick,
                             bool is_moving) {
    int dir_idx = direction_to_index(dir);
    int frame   = is_moving ? frame_for_tick(tick, dir_idx) : 0;

    const SDL2pp::Rect& body_src = _body_anims[dir_idx].frames[frame];
    const SDL2pp::Rect& head_src = _head_rects[dir_idx];

    SDL2pp::Rect body_dst(
        screen_x - BODY_W / 2 + TILE_SIZE / 2,
        screen_y - BODY_H + TILE_SIZE,
        BODY_W,
        BODY_H
    );

    // Overlap por dirección: sur=10, norte=10, este=8, oeste=12
    int overlaps[4] = {10, 10, 8, 12};
    int overlap = overlaps[dir_idx];

    SDL2pp::Rect head_dst(
        body_dst.x,
        body_dst.y - head_src.h + overlap,
        head_src.w,
        head_src.h
    );

    SDL2pp::Texture& body_tex = assets.get(body_path);
    SDL2pp::Texture& head_tex = assets.get(head_path);

    renderer.Copy(body_tex, body_src, body_dst);
    renderer.Copy(head_tex, head_src, head_dst);
}