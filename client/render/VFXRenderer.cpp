#include "VFXRenderer.h"

#include <algorithm>
#include <string>

#include "../config/ClientConfig.h"
#include "../config/SpellVfxConfig.h"
#include "../config/UiConstants.h"

VFXRenderer::VFXRenderer(SDL2pp::Renderer& renderer, Camera& camera, AssetManager& assets):
        _renderer(renderer), _camera(camera), _assets(assets) {}

void VFXRenderer::render_spells(WorldState& state) {
    int ticks_per_frame = ClientConfig::instance().rendering.spell_ticks_per_frame;

    state.spell_effects.erase(
            std::remove_if(state.spell_effects.begin(), state.spell_effects.end(),
                           [&](const SpellEffect& fx) {
                               int total = static_cast<int>(fx.frame_indices.size());
                               uint32_t elapsed = state.current_tick - fx.start_tick;
                               return static_cast<int>(elapsed / ticks_per_frame) >= total;
                           }),
            state.spell_effects.end());

    for (const auto& fx: state.spell_effects) {
        int total = static_cast<int>(fx.frame_indices.size());
        if (total == 0 || fx.sheet_cols <= 0 || fx.frame_w <= 0 || fx.frame_h <= 0)
            continue;

        uint32_t elapsed = state.current_tick - fx.start_tick;
        int frame = static_cast<int>(elapsed / ticks_per_frame) % total;
        int sheet_frame = fx.frame_indices[frame];

        int col = sheet_frame % fx.sheet_cols;
        int row = sheet_frame / fx.sheet_cols;

        SDL2pp::Texture& tex = _assets.get(fx.path);
        SDL2pp::Rect src(col * fx.frame_w, row * fx.frame_h, fx.frame_w, fx.frame_h);

        int center_x = _camera.world_to_screen_x(static_cast<float>(
                               fx.pos_x * ClientConfig::instance().tile_size())) +
                       ClientConfig::instance().tile_size() / 2;
        int center_y = _camera.world_to_screen_y(static_cast<float>(
                               fx.pos_y * ClientConfig::instance().tile_size())) +
                       ClientConfig::instance().tile_size() / 2;

        const auto& render = SpellVfxConfig::instance().get_render_info(fx.spell_id);
        int dw = render.display_w, dh = render.display_h;
        int ox = render.offset_x, oy = render.offset_y;
        if (dw <= 0 || dh <= 0) {
            dw = ClientConfig::instance().tile_size() * 2;
            dh = ClientConfig::instance().tile_size() * 2;
            ox = -ClientConfig::instance().tile_size() / 2;
            oy = -ClientConfig::instance().tile_size();
        }

        SDL2pp::Rect dst(center_x + ox, center_y + oy, dw, dh);
        _renderer.Copy(tex, src, dst);
    }
}

void VFXRenderer::render_projectiles(WorldState& state) {
    uint32_t duration_ticks = ClientConfig::instance().projectiles.duration_ticks;

    state.projectiles.erase(std::remove_if(state.projectiles.begin(), state.projectiles.end(),
                                           [&](const Projectile& p) {
                                               return (state.current_tick - p.start_tick) >=
                                                      duration_ticks;
                                           }),
                            state.projectiles.end());

    for (const auto& p: state.projectiles) {
        float t = static_cast<float>(state.current_tick - p.start_tick) /
                  static_cast<float>(duration_ticks);
        t = std::clamp(t, 0.0f, 1.0f);

        float world_x =
                (p.from_x + (p.to_x - p.from_x) * t) * ClientConfig::instance().tile_size() +
                ClientConfig::instance().tile_size() / 2.0f;
        float world_y =
                (p.from_y + (p.to_y - p.from_y) * t) * ClientConfig::instance().tile_size() +
                ClientConfig::instance().tile_size() / 2.0f;

        int sx = _camera.world_to_screen_x(world_x);
        int sy = _camera.world_to_screen_y(world_y);

        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_Color color = p.is_magic ? UI_PROJECTILE_MAGIC_COLOR : UI_PROJECTILE_PHYSICAL_COLOR;
        SDL_SetRenderDrawColor(_renderer.Get(), color.r, color.g, color.b, color.a);
        SDL_Rect dot{sx - 3, sy - 3, 6, 6};
        SDL_RenderFillRect(_renderer.Get(), &dot);
    }
}

void VFXRenderer::render_deaths(WorldState& state) {
    const auto& death_cfg = ClientConfig::instance().death_effects;
    uint32_t now = SDL_GetTicks();

    state.death_effects.erase(std::remove_if(state.death_effects.begin(), state.death_effects.end(),
                                             [&](const DeathEffect& d) {
                                                 return (now - d.start_ms) >=
                                                        death_cfg.death_duration_ms;
                                             }),
                              state.death_effects.end());

    for (const auto& d: state.death_effects) {
        uint32_t elapsed_ms = now - d.start_ms;
        int frame = std::clamp(static_cast<int>(elapsed_ms / death_cfg.death_frame_ms), 0,
                               death_cfg.death_frames - 1);

        std::string path = death_cfg.sprite_base_path + std::to_string(frame + 1) + ".png";
        int sx = _camera.tile_to_screen_x(static_cast<int>(d.pos_x));
        int sy = _camera.tile_to_screen_y(static_cast<int>(d.pos_y));
        SDL2pp::Rect dst(sx, sy, ClientConfig::instance().tile_size(),
                         ClientConfig::instance().tile_size());
        _renderer.Copy(_assets.get(path), SDL2pp::NullOpt, dst);
    }
}
