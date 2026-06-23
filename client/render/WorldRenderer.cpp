#include "WorldRenderer.h"

#include <algorithm>

#include "../config/ClientConfig.h"
#include "../config/ItemVisualConfig.h"
#include "../ui/InventoryPanel.h"

namespace {
// Los offset_x/offset_y de tiles.toml y objects_sup.toml están calculados en
// píxeles para un tile de 64px (el tile_size original, antes de los ajustes
// de zoom de cámara). Si tile_size cambia, hay que escalarlos en la misma
// proporción o quedan desalineados (ver "la ola"/costa y los acantilados).
constexpr float REFERENCE_TILE_SIZE = 64.0f;

int scaled_offset(int offset) {
    return static_cast<int>(offset * (ClientConfig::instance().tile_size() / REFERENCE_TILE_SIZE));
}
}  // namespace

WorldRenderer::WorldRenderer(SDL2pp::Window& window, SDL2pp::Renderer& renderer, Camera& camera):
        _window(window),
        _renderer(renderer),
        _camera(camera),
        _assets(renderer),
        _tile_config("config/tiles.toml", "floor"),
        _obj_sup_config("config/objects_sup.toml"),
        _entity_renderer(renderer, camera, _assets),
        _vfx_renderer(renderer, camera, _assets) {}

void WorldRenderer::load_item_textures(InventoryPanel& inventory) {
    for (const auto& [item_id, entry]: ItemVisualConfig::instance().all_items()) {
        if (entry.icon_path.empty())
            continue;
        try {
            SDL2pp::Texture& tex = _assets.get(entry.icon_path);
            inventory.register_item_texture(item_id, tex.Get());
        } catch (...) {
            // Si el asset no existe, el slot mostrará la abreviatura en texto
        }
    }
}

void WorldRenderer::render(WorldState& state, const PlayerState& player) {
    render_floor(state);
    _entity_renderer.render(state, player);
    _vfx_renderer.render_deaths(state);
    render_obj_sup(state);
    _vfx_renderer.render_spells(state);
    _vfx_renderer.render_projectiles(state);
}

void WorldRenderer::render_floor(const WorldState& state) {
    int screen_w = _window.GetWidth();
    int screen_h = _window.GetHeight();
    int map_size = ClientConfig::instance().rendering.map_size;
    int map_w = state.map_loaded ? state.map.width : map_size;
    int map_h = state.map_loaded ? state.map.height : map_size;

    int margin = 8;
    int first_x = std::max(
            0, -_camera.tile_to_screen_x(0) / ClientConfig::instance().tile_size() - margin);
    int first_y = std::max(
            0, -_camera.tile_to_screen_y(0) / ClientConfig::instance().tile_size() - margin);
    int last_x = std::min(map_w - 1,
                          first_x + screen_w / ClientConfig::instance().tile_size() + margin * 2);
    int last_y = std::min(map_h - 1,
                          first_y + screen_h / ClientConfig::instance().tile_size() + margin * 2);

    for (int ty = first_y; ty <= last_y; ty++) {
        for (int tx = first_x; tx <= last_x; tx++) {
            uint16_t floor_id = 0;
            if (state.map_loaded)
                floor_id = state.map.tiles[ty * state.map.width + tx].floor_id;

            const TileEntry& entry = _tile_config.get(floor_id);
            if (entry.is_large())
                continue;

            int sx = _camera.tile_to_screen_x(tx);
            int sy = _camera.tile_to_screen_y(ty);

            if (entry.path.empty()) {
                if (sx >= -ClientConfig::instance().tile_size() && sx <= screen_w &&
                    sy >= -ClientConfig::instance().tile_size() && sy <= screen_h) {
                    _renderer.SetDrawColor(0, 0, 0, 255);
                    _renderer.FillRect(SDL2pp::Rect(sx, sy, ClientConfig::instance().tile_size(),
                                                    ClientConfig::instance().tile_size()));
                }
                continue;
            }

            SDL2pp::Rect dst(sx, sy, ClientConfig::instance().tile_size(),
                             ClientConfig::instance().tile_size());
            if (entry.has_src_rect()) {
                SDL2pp::Rect src(entry.src_x, entry.src_y, entry.src_w, entry.src_h);
                _renderer.Copy(_assets.get(entry.path), src, dst);
            } else {
                _renderer.Copy(_assets.get(entry.path), SDL2pp::NullOpt, dst);
            }
        }
    }

    int obj_sup_size = ClientConfig::instance().rendering.obj_sup_size;
    for (int ty = 0; ty < map_h; ty++) {
        int sy_raw = _camera.tile_to_screen_y(ty);
        if (sy_raw > screen_h + obj_sup_size)
            continue;
        if (sy_raw < -(obj_sup_size + obj_sup_size))
            continue;

        for (int tx = 0; tx < map_w; tx++) {
            uint16_t floor_id = 0;
            if (state.map_loaded)
                floor_id = state.map.tiles[ty * state.map.width + tx].floor_id;

            const TileEntry& entry = _tile_config.get(floor_id);
            if (!entry.is_large())
                continue;

            int sx = _camera.tile_to_screen_x(tx);
            if (sx > screen_w + obj_sup_size)
                continue;
            if (sx < -(obj_sup_size + obj_sup_size))
                continue;

            int size_px = entry.tile_size * ClientConfig::instance().tile_size();
            SDL2pp::Rect dst(sx + scaled_offset(entry.offset_x),
                             sy_raw + scaled_offset(entry.offset_y), size_px, size_px);
            _renderer.Copy(_assets.get(entry.path), SDL2pp::NullOpt, dst);
        }
    }
}

void WorldRenderer::render_obj_sup(const WorldState& state) {
    if (!state.map_loaded)
        return;

    int screen_w = _window.GetWidth();
    int screen_h = _window.GetHeight();
    int map_w = state.map.width;
    int map_h = state.map.height;

    int margin_x = 8;
    int margin_y_up = 6;
    int margin_y_down = 16;
    int first_x = std::max(
            0, -_camera.tile_to_screen_x(0) / ClientConfig::instance().tile_size() - margin_x);
    int first_y = std::max(
            0, -_camera.tile_to_screen_y(0) / ClientConfig::instance().tile_size() - margin_y_up);
    int last_x = std::min(map_w - 1,
                          first_x + screen_w / ClientConfig::instance().tile_size() + margin_x * 2);
    int last_y = std::min(map_h - 1, first_y + screen_h / ClientConfig::instance().tile_size() +
                                             margin_y_up + margin_y_down);

    for (int ty = first_y; ty <= last_y; ty++) {
        for (int tx = first_x; tx <= last_x; tx++) {
            uint16_t obj_id = state.map.tiles[ty * map_w + tx].object_superior_id;
            if (obj_id == 0)
                continue;

            const ObjectSupEntry& entry = _obj_sup_config.get(obj_id);
            if (entry.frames.empty())
                continue;

            int frame_idx = (state.current_tick /
                             ClientConfig::instance().rendering.obj_sup_ticks_per_frame) %
                            static_cast<int>(entry.frames.size());

            int obj_h = entry.size_tiles * ClientConfig::instance().tile_size();
            int obj_w = entry.width_tiles * ClientConfig::instance().tile_size();

            int sx = _camera.tile_to_screen_x(tx);
            int sy = _camera.tile_to_screen_y(ty);

            SDL2pp::Rect dst(sx - (obj_w - ClientConfig::instance().tile_size()) / 2 +
                                     scaled_offset(entry.offset_x),
                             sy - obj_h + ClientConfig::instance().tile_size() +
                                     scaled_offset(entry.offset_y),
                             obj_w, obj_h);

            _renderer.Copy(_assets.get(entry.frames[frame_idx]), SDL2pp::NullOpt, dst);
        }
    }
}
