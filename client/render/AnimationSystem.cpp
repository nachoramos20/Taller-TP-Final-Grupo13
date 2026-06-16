#include "AnimationSystem.h"
#include <algorithm>

AnimationSystem::AnimationSystem()
    : _last_sprite_id(-1),
      _head_overlaps{38, 40, 38, 38} {}

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

int AnimationSystem::frame_for_tick(uint32_t tick, int n_frames) const {
    return (tick / TICKS_PER_FRAME) % n_frames;
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

void AnimationSystem::render_weapon(SDL2pp::Renderer& renderer,
                                    AssetManager& assets,
                                    const std::string& weapon_path,
                                    int dir_idx, int frame,
                                    const SDL2pp::Rect& body_dst) {
    if (weapon_path.empty()) return;

    static constexpr int WPN_ROW_H = 48;
    SDL2pp::Rect wpn_src(frame * BodyLayout::FRAME_W, dir_idx * WPN_ROW_H,
                         BodyLayout::FRAME_W, WPN_ROW_H);

    static constexpr int WPN_SIZE = 24;
    int wx, wy;
    switch (dir_idx) {
        case DIR_WEST:
            wx = body_dst.x - WPN_SIZE;
            wy = body_dst.y + body_dst.h - WPN_SIZE;
            break;
        default:
            wx = body_dst.x + body_dst.w;
            wy = body_dst.y + body_dst.h - WPN_SIZE;
            break;
    }
    SDL2pp::Rect wpn_dst(wx, wy, WPN_SIZE, WPN_SIZE);
    renderer.Copy(assets.get(weapon_path), wpn_src, wpn_dst);
}

SpriteBounds AnimationSystem::render(SDL2pp::Renderer& renderer,
                             AssetManager& assets,
                             const std::string& body_path,
                             const std::string& head_path,
                             uint8_t sprite_id,
                             Direction dir,
                             int screen_x, int screen_y,
                             uint32_t tick,
                             bool is_moving,
                             const EquipVisual* equip,
                             bool is_ghost) {
    // Las texturas están cacheadas por path: el alpha-mod aplicado acá afecta
    // a la misma textura compartida, por eso se restaura a opaco al final
    // (sino el "fantasma" de uno contagiaría la transparencia a otro sprite
    // que reuse la misma imagen más adelante en el frame).
    static constexpr Uint8 GHOST_ALPHA = 120;
    std::vector<SDL2pp::Texture*> ghost_textures;
    auto get_tex = [&](const std::string& path) -> SDL2pp::Texture& {
        SDL2pp::Texture& tex = assets.get(path);
        if (is_ghost) {
            tex.SetAlphaMod(GHOST_ALPHA);
            ghost_textures.push_back(&tex);
        }
        return tex;
    };
    int dir_idx = direction_to_index(dir);
    int n = static_cast<int>(_body_anims[dir_idx].frames.size());
    int frame = is_moving ? frame_for_tick(tick, n) : 0;

    if (sprite_id != static_cast<uint8_t>(_last_sprite_id)) {
        HeadLayout hl = HeadLayout::for_sprite(sprite_id);
        for (int d = 0; d < 4; d++) {
            _head_rects[d]    = SDL2pp::Rect(0, hl.dir_y[d], hl.width, hl.dir_h[d]);
            _head_overlaps[d] = hl.overlaps[d];
        }
        _last_sprite_id = sprite_id;
    }

    const SDL2pp::Rect& body_src = _body_anims[dir_idx].frames[frame];
    const SDL2pp::Rect& head_src = _head_rects[dir_idx];

    SDL2pp::Rect body_dst(
        screen_x - BodyLayout::FRAME_W / 2 + TILE_SIZE / 2,
        screen_y - body_src.h + TILE_SIZE,
        BodyLayout::FRAME_W,
        body_src.h
    );
    SDL2pp::Rect head_dst(
        body_dst.x + (BodyLayout::FRAME_W - head_src.w) / 2,
        body_dst.y - head_src.h + _head_overlaps[dir_idx],
        head_src.w,
        head_src.h
    );

    // Pasada 1: body base
    renderer.Copy(get_tex(body_path), body_src, body_dst);

    // Pasada 2: armadura superpuesta
    if (equip && !equip->armor_path.empty())
        renderer.Copy(get_tex(equip->armor_path), body_src, body_dst);

    // Pasada 3: cabeza
    renderer.Copy(get_tex(head_path), head_src, head_dst);

    // Pasada 4: casco
    if (equip && !equip->helmet_path.empty() && equip->helmet_src_w > 0) {
        SDL2pp::Rect helm_src(equip->helmet_src_x, equip->helmet_src_y,
                              equip->helmet_src_w, equip->helmet_src_h);
        SDL2pp::Rect helm_dst(
            head_dst.x + (head_dst.w - equip->helmet_src_w) / 2,
            head_dst.y,
            equip->helmet_src_w,
            equip->helmet_src_h
        );
        renderer.Copy(get_tex(equip->helmet_path), helm_src, helm_dst);
    }

    // Pasada 5: escudo en lado izquierdo
    if (equip && !equip->shield_path.empty()) {
        static constexpr int SHIELD_W = 24;
        static constexpr int SHIELD_H = 32;

        int sx = body_dst.x - SHIELD_W - 2;
        int sy = body_dst.y + body_dst.h - SHIELD_H;

        SDL2pp::Rect shield_src(0, 0, SHIELD_W, SHIELD_H);
        SDL2pp::Rect shield_dst(sx, sy, SHIELD_W, SHIELD_H);
        renderer.Copy(get_tex(equip->shield_path), shield_src, shield_dst);
    }

    // Pasada 6: arma en mano
    if (equip) {
        if (is_ghost && !equip->weapon_path.empty()) get_tex(equip->weapon_path);
        render_weapon(renderer, assets, equip->weapon_path, dir_idx, frame, body_dst);
    }

    for (SDL2pp::Texture* tex : ghost_textures)
        tex->SetAlphaMod(255);

    return { head_dst.y, body_dst.x + body_dst.w / 2, body_dst.w };
}

SpriteBounds AnimationSystem::render_npc(SDL2pp::Renderer& renderer,
                                  AssetManager& assets,
                                  const std::string& sheet_path,
                                  int cols, int rows,
                                  int frame_w, int frame_h,
                                  Direction dir,
                                  int screen_x, int screen_y,
                                  uint32_t tick,
                                  bool is_moving,
                                  float scale,
                                  int draw_offset_y) {
    int dir_idx = std::min(direction_to_index(dir), rows - 1);
    int frame   = is_moving ? frame_for_tick(tick, cols) : 0;

    SDL2pp::Rect src(frame * frame_w, dir_idx * frame_h, frame_w, frame_h);

    int dst_h = static_cast<int>(TILE_SIZE * scale);
    int dst_w = (frame_h > 0) ? (dst_h * frame_w / frame_h) : dst_h;

    int top_y = screen_y + TILE_SIZE - dst_h;

    SDL2pp::Rect dst(
        screen_x + (TILE_SIZE - dst_w) / 2,
        top_y + draw_offset_y,
        dst_w,
        dst_h
    );

    renderer.Copy(assets.get(sheet_path), src, dst);

    return { top_y, dst.x + dst.w / 2, dst.w };
}
