#include "AnimationSystem.h"
#include "../config/ClientConfig.h"
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

static constexpr int WPN_ROW_H = 48;

static bool weapon_on_left(int dir_idx) {
    return dir_idx == AnimationSystem::DIR_WEST || dir_idx == AnimationSystem::DIR_SOUTH;
}

static bool needs_north_south_swap(const std::string& weapon_path) {
    return weapon_path.find("/bow/") != std::string::npos;
}

static int weapon_row(int dir_idx, const std::string& weapon_path) {
    if (!needs_north_south_swap(weapon_path)) return dir_idx;
    if (dir_idx == AnimationSystem::DIR_SOUTH) return AnimationSystem::DIR_NORTH;
    if (dir_idx == AnimationSystem::DIR_NORTH) return AnimationSystem::DIR_SOUTH;
    return dir_idx;
}

void AnimationSystem::render_weapon(SDL2pp::Renderer& renderer,
                                    AssetManager& assets,
                                    const std::string& weapon_path,
                                    int dir_idx, int frame,
                                    const SDL2pp::Rect& body_dst,
                                    float scale) {
    if (weapon_path.empty()) return;

    SDL2pp::Rect wpn_src(frame * BodyLayout::FRAME_W, weapon_row(dir_idx, weapon_path) * WPN_ROW_H,
                         BodyLayout::FRAME_W, WPN_ROW_H);

    int wpn_size = static_cast<int>(24 * scale);
    int overlap  = static_cast<int>(15 * scale);  // se mete en el cuerpo, así parece que la tiene en la mano
    int wx, wy;
    if (weapon_on_left(dir_idx)) {
        wx = body_dst.x - wpn_size + overlap;
        wy = body_dst.y + body_dst.h - wpn_size;
    } else {
        wx = body_dst.x + body_dst.w - overlap;
        wy = body_dst.y + body_dst.h - wpn_size;
    }
    SDL2pp::Rect wpn_dst(wx, wy, wpn_size, wpn_size);
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
            _head_offset_x[d] = hl.offset_x[d];
        }
        _armor_scale_y = hl.armor_scale_y;
        _last_sprite_id = sprite_id;
    }

    const SDL2pp::Rect& body_src = _body_anims[dir_idx].frames[frame];
    const SDL2pp::Rect& head_src = _head_rects[dir_idx];

    // Escala el personaje en la misma proporción que el tile_size.
    float scale = ClientConfig::instance().tile_size() / 64.0f;
    int body_w = static_cast<int>(BodyLayout::FRAME_W * scale);
    int body_h = static_cast<int>(body_src.h * scale);

    SDL2pp::Rect body_dst(
        screen_x - body_w / 2 + ClientConfig::instance().tile_size() / 2,
        screen_y - body_h + ClientConfig::instance().tile_size(),
        body_w,
        body_h
    );
    int head_w = static_cast<int>(head_src.w * scale);
    int head_h = static_cast<int>(head_src.h * scale);
    SDL2pp::Rect head_dst(
        body_dst.x + (body_dst.w - head_w) / 2 + static_cast<int>(_head_offset_x[dir_idx] * scale),
        body_dst.y - head_h + static_cast<int>(_head_overlaps[dir_idx] * scale),
        head_w,
        head_h
    );

    auto draw_shield = [&]() {
        if (!equip || equip->shield_path.empty()) return;
        SDL2pp::Rect shield_src(0, dir_idx * WPN_ROW_H, BodyLayout::FRAME_W, WPN_ROW_H);

        int shield_w    = static_cast<int>(24 * scale);
        int shield_h     = static_cast<int>(34 * scale);
        int overlap     = static_cast<int>(15 * scale);

        // Va siempre del lado contrario al arma.
        int sx = weapon_on_left(dir_idx)
            ? body_dst.x + body_dst.w - overlap
            : body_dst.x - shield_w + overlap;
        int sy = body_dst.y + body_dst.h - shield_h;

        SDL2pp::Rect shield_dst(sx, sy, shield_w, shield_h);
        renderer.Copy(get_tex(equip->shield_path), shield_src, shield_dst);
    };
    auto draw_weapon = [&]() {
        if (!equip) return;
        if (is_ghost && !equip->weapon_path.empty()) get_tex(equip->weapon_path);
        render_weapon(renderer, assets, equip->weapon_path, dir_idx, frame, body_dst, scale);
    };

    bool weapon_behind = (dir_idx == DIR_WEST || dir_idx == DIR_NORTH);
    bool shield_behind = (dir_idx == DIR_EAST || dir_idx == DIR_NORTH);

    if (weapon_behind) draw_weapon();
    if (shield_behind) draw_shield();

    // Pasada 1: body base
    renderer.Copy(get_tex(body_path), body_src, body_dst);

    // Pasada 2: armadura superpuesta
    if (equip && !equip->armor_path.empty()) {
        int armor_h = static_cast<int>(body_dst.h * _armor_scale_y);
        SDL2pp::Rect armor_dst(
            body_dst.x,
            body_dst.y + body_dst.h - armor_h,
            body_dst.w,
            armor_h
        );
        renderer.Copy(get_tex(equip->armor_path), body_src, armor_dst);
    }

    // Pasada 3: cabeza
    renderer.Copy(get_tex(head_path), head_src, head_dst);

    // Pasada 4: casco
    if (equip && !equip->helmet_path.empty() && equip->helmet_src_w > 0) {
        SDL2pp::Rect helm_src(equip->helmet_src_x, dir_idx * equip->helmet_src_h,
                              equip->helmet_src_w, equip->helmet_src_h);
        int helm_w = static_cast<int>(equip->helmet_src_w * scale);
        int helm_h = static_cast<int>(equip->helmet_src_h * scale);
        SDL2pp::Rect helm_dst(
            head_dst.x + (head_dst.w - helm_w) / 2 + static_cast<int>(equip->helmet_offset_x[dir_idx] * scale),
            head_dst.y + static_cast<int>(equip->helmet_offset_y[dir_idx] * scale),
            helm_w,
            helm_h
        );
        renderer.Copy(get_tex(equip->helmet_path), helm_src, helm_dst);
    }

    // Pasada 5/6: escudo y arma del lado de la cámara 
    if (!shield_behind) draw_shield();
    if (!weapon_behind) draw_weapon();

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

    int dst_h = static_cast<int>(ClientConfig::instance().tile_size() * scale);
    int dst_w = (frame_h > 0) ? (dst_h * frame_w / frame_h) : dst_h;

    int top_y = screen_y + ClientConfig::instance().tile_size() - dst_h;

    SDL2pp::Rect dst(
        screen_x + (ClientConfig::instance().tile_size() - dst_w) / 2,
        top_y + draw_offset_y,
        dst_w,
        dst_h
    );

    renderer.Copy(assets.get(sheet_path), src, dst);

    return { top_y, dst.x + dst.w / 2, dst.w };
}
