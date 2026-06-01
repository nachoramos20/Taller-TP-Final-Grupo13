#include "AnimationSystem.h"

AnimationSystem::AnimationSystem() : _last_sprite_id(-1) {}

void AnimationSystem::load() {
    for (int dir = 0; dir < 4; dir++) {
        _body_anims[dir].frames.clear();
        for (int f = 0; f < BodyLayout::DIR_N[dir]; f++) {
            _body_anims[dir].frames.push_back(SDL2pp::Rect(
                f * BodyLayout::FRAME_W,
                BodyLayout::DIR_Y[dir],
                BodyLayout::FRAME_W,
                BodyLayout::DIR_H[dir]
            ));
        }
    }
}

int AnimationSystem::frame_for_tick(uint32_t tick, int dir_idx) const {
    int n = static_cast<int>(_body_anims[dir_idx].frames.size());
    return (tick / TICKS_PER_FRAME) % n;
}

int AnimationSystem::direction_to_index(Direction dir) const {
    switch (dir) {
        case Direction::SOUTH: return DIR_SOUTH;
        case Direction::NORTH: return DIR_NORTH;
        case Direction::WEST:  return DIR_WEST;
        case Direction::EAST:  return DIR_EAST;
        default:               return DIR_SOUTH;
    }
}

void AnimationSystem::render(SDL2pp::Renderer& renderer,
                             AssetManager& assets,
                             const std::string& body_path,
                             const std::string& head_path,
                             uint8_t sprite_id,
                             Direction dir,
                             int screen_x, int screen_y,
                             uint32_t tick,
                             bool is_moving) {
    int dir_idx = direction_to_index(dir);
    int frame   = is_moving ? frame_for_tick(tick, dir_idx) : 0;

    // Recalcular head rects si cambió el sprite_id
    if (sprite_id != static_cast<uint8_t>(_last_sprite_id)) {
        HeadLayout hl = HeadLayout::for_sprite(sprite_id);
        for (int d = 0; d < 4; d++) {
            _head_rects[d] = SDL2pp::Rect(0, hl.dir_y[d], hl.width, hl.dir_h[d]);
        }
        _last_sprite_id = sprite_id;
    }

    const SDL2pp::Rect& body_src = _body_anims[dir_idx].frames[frame];
    const SDL2pp::Rect& head_src = _head_rects[dir_idx];

    // Body: centrado en el tile, pies abajo
    SDL2pp::Rect body_dst(
        screen_x - BodyLayout::FRAME_W / 2 + TILE_SIZE / 2,
        screen_y - body_src.h + TILE_SIZE,
        BodyLayout::FRAME_W,
        body_src.h
    );

    // Head: encima del body con overlap por dirección
    int overlaps[4] = {10, 10, 8, 12};
    SDL2pp::Rect head_dst(
        body_dst.x + (BodyLayout::FRAME_W - head_src.w) / 2,
        body_dst.y - head_src.h + overlaps[dir_idx],
        head_src.w,
        head_src.h
    );

    renderer.Copy(assets.get(body_path), body_src, body_dst);
    renderer.Copy(assets.get(head_path), head_src, head_dst);
}