#include "WorldRenderer.h"
#include <algorithm>
#include "../ui/InventoryPanel.h"
#include "../config/ClientConfig.h"
#include "../config/SpellVfxConfig.h"
#include "../config/ItemVisualConfig.h"
#include "../config/NpcVisualConfig.h"

namespace {
// Los offset_x/offset_y de tiles.toml y objects_sup.toml están calculados en
// píxeles para un tile de 64px (el tile_size original, antes de los ajustes
// de zoom de cámara). Si tile_size cambia, hay que escalarlos en la misma
// proporción o quedan desalineados (ver "la ola"/costa y los acantilados).
constexpr float REFERENCE_TILE_SIZE = 64.0f;

int scaled_offset(int offset) {
    return static_cast<int>(offset * (ClientConfig::instance().tile_size() / REFERENCE_TILE_SIZE));
}
}

WorldRenderer::WorldRenderer(SDL2pp::Window& window, SDL2pp::Renderer& renderer, Camera& camera)
    : _window(window), _renderer(renderer), _camera(camera),
      _assets(renderer),
      _sprite_config("config/sprites.toml"),
      _tile_config("config/tiles.toml", "floor"),
      _obj_sup_config("config/objects_sup.toml") {
    _anim.load();
}

void WorldRenderer::load_item_textures(InventoryPanel& inventory) {
    for (const auto& [item_id, entry] : ItemVisualConfig::instance().all_items()) {
        if (entry.icon_path.empty()) continue;
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
    render_entities(state, player);
    render_deaths(state);
    render_obj_sup(state);
    render_spells(state);
    render_projectiles(state);
}

void WorldRenderer::render_floor(const WorldState& state) {
    int screen_w = _window.GetWidth();
    int screen_h = _window.GetHeight();
    int map_size = ClientConfig::instance().rendering.map_size;
    int map_w = state.map_loaded ? state.map.width  : map_size;
    int map_h = state.map_loaded ? state.map.height : map_size;

    int margin = 8;
    int first_x = std::max(0, -_camera.tile_to_screen_x(0) / ClientConfig::instance().tile_size() - margin);
    int first_y = std::max(0, -_camera.tile_to_screen_y(0) / ClientConfig::instance().tile_size() - margin);
    int last_x  = std::min(map_w - 1, first_x + screen_w / ClientConfig::instance().tile_size() + margin * 2);
    int last_y  = std::min(map_h - 1, first_y + screen_h / ClientConfig::instance().tile_size() + margin * 2);

    for (int ty = first_y; ty <= last_y; ty++) {
        for (int tx = first_x; tx <= last_x; tx++) {
            uint16_t floor_id = 0;
            if (state.map_loaded)
                floor_id = state.map.tiles[ty * state.map.width + tx].floor_id;

            const TileEntry& entry = _tile_config.get(floor_id);
            if (entry.is_large()) continue;

            int sx = _camera.tile_to_screen_x(tx);
            int sy = _camera.tile_to_screen_y(ty);

            if (entry.path.empty()) {
                if (sx >= -ClientConfig::instance().tile_size() && sx <= screen_w &&
                    sy >= -ClientConfig::instance().tile_size() && sy <= screen_h) {
                    _renderer.SetDrawColor(0, 0, 0, 255);
                    _renderer.FillRect(SDL2pp::Rect(sx, sy, ClientConfig::instance().tile_size(), ClientConfig::instance().tile_size()));
                }
                continue;
            }

            SDL2pp::Rect dst(sx, sy, ClientConfig::instance().tile_size(), ClientConfig::instance().tile_size());
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
        if (sy_raw > screen_h + obj_sup_size) continue;
        if (sy_raw < -(obj_sup_size + obj_sup_size))   continue;

        for (int tx = 0; tx < map_w; tx++) {
            uint16_t floor_id = 0;
            if (state.map_loaded)
                floor_id = state.map.tiles[ty * state.map.width + tx].floor_id;

            const TileEntry& entry = _tile_config.get(floor_id);
            if (!entry.is_large()) continue;

            int sx = _camera.tile_to_screen_x(tx);
            if (sx > screen_w + obj_sup_size) continue;
            if (sx < -(obj_sup_size + obj_sup_size))   continue;

            int size_px = entry.tile_size * ClientConfig::instance().tile_size();
            SDL2pp::Rect dst(sx + scaled_offset(entry.offset_x), sy_raw + scaled_offset(entry.offset_y),
                              size_px, size_px);
            _renderer.Copy(_assets.get(entry.path), SDL2pp::NullOpt, dst);
        }
    }
}

void WorldRenderer::render_obj_sup(const WorldState& state) {
    if (!state.map_loaded) return;

    int screen_w = _window.GetWidth();
    int screen_h = _window.GetHeight();
    int map_w = state.map.width;
    int map_h = state.map.height;

    int margin_x = 8;
    int margin_y_up   = 6;
    int margin_y_down = 16;
    int first_x = std::max(0, -_camera.tile_to_screen_x(0) / ClientConfig::instance().tile_size() - margin_x);
    int first_y = std::max(0, -_camera.tile_to_screen_y(0) / ClientConfig::instance().tile_size() - margin_y_up);
    int last_x  = std::min(map_w - 1, first_x + screen_w / ClientConfig::instance().tile_size() + margin_x * 2);
    int last_y  = std::min(map_h - 1, first_y + screen_h / ClientConfig::instance().tile_size() + margin_y_up + margin_y_down);

    for (int ty = first_y; ty <= last_y; ty++) {
        for (int tx = first_x; tx <= last_x; tx++) {
            uint16_t obj_id = state.map.tiles[ty * map_w + tx].object_superior_id;
            if (obj_id == 0) continue;

            const ObjectSupEntry& entry = _obj_sup_config.get(obj_id);
            if (entry.frames.empty()) continue;

            int frame_idx = (state.current_tick / ClientConfig::instance().rendering.obj_sup_ticks_per_frame)
                            % static_cast<int>(entry.frames.size());

            int obj_h = entry.size_tiles  * ClientConfig::instance().tile_size();
            int obj_w = entry.width_tiles * ClientConfig::instance().tile_size();

            int sx = _camera.tile_to_screen_x(tx);
            int sy = _camera.tile_to_screen_y(ty);

            SDL2pp::Rect dst(
                sx - (obj_w - ClientConfig::instance().tile_size()) / 2 + scaled_offset(entry.offset_x),
                sy - obj_h + ClientConfig::instance().tile_size() + scaled_offset(entry.offset_y),
                obj_w,
                obj_h
            );

            _renderer.Copy(_assets.get(entry.frames[frame_idx]), SDL2pp::NullOpt, dst);
        }
    }
}

void WorldRenderer::render_floor_item(const EntityDTO& e, int screen_x, int screen_y) {
    if (e.sprite_id == static_cast<uint8_t>(ItemId::BLOOD_STAIN)) return;

    const auto& variants = ItemVisualConfig::instance().get_floor_variants(e.sprite_id);
    uint8_t variant_idx = e.direction % static_cast<uint8_t>(variants.size());
    const std::string& path = variants[variant_idx];

    SDL2pp::Rect dst(screen_x, screen_y, ClientConfig::instance().tile_size(), ClientConfig::instance().tile_size());
    SDL2pp::Texture& tex = _assets.get(path);
    if (tex.GetWidth() == 256 && tex.GetHeight() == 256)
        _renderer.Copy(tex, SDL2pp::Rect(0, 192, 48, 64), dst);
    else
        _renderer.Copy(tex, SDL2pp::NullOpt, dst);
}

void WorldRenderer::render_npc(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
                                Direction dir, bool moving) {
    const auto& npc_entry = NpcVisualConfig::instance().get(e.sprite_id);
    if (npc_entry.variants.empty()) return;

    const NpcSheetVariant& variant =
        NpcVisualConfig::instance().select_variant(e.sprite_id, e.entity_id, e.pos_y);

    SpriteBounds bounds = _anim.render_npc(_renderer, _assets,
                     variant.sheet_path, variant.cols, variant.rows, variant.frame_w, variant.frame_h,
                     dir, screen_x, screen_y,
                     state.current_tick, moving, npc_entry.scale, variant.draw_offset_y);
    render_entity_healthbar(e, bounds, npc_entry.is_service, variant.name_label_offset_y);
}

EquipVisual WorldRenderer::build_equip_visual(uint8_t item_weapon, uint8_t item_armor, uint8_t item_helmet,
                                               uint8_t item_shield, uint8_t race_sprite_id) const {
    EquipVisual equip{};
    auto& cfg = ItemVisualConfig::instance();

    if (item_weapon != 0) equip.weapon_path = cfg.get(item_weapon).equip_path;
    if (item_armor  != 0) equip.armor_path  = cfg.get(item_armor).equip_path;
    if (item_shield != 0) equip.shield_path = cfg.get(item_shield).equip_path;

    if (item_helmet != 0) {
        if (const HelmetVisual* helmet = cfg.get_helmet(item_helmet)) {
            equip.helmet_path  = helmet->path;
            equip.helmet_src_x = helmet->src_x;
            equip.helmet_src_y = helmet->src_y;
            equip.helmet_src_w = helmet->src_w;
            equip.helmet_src_h = helmet->src_h;
            int race = (race_sprite_id >= 1 && race_sprite_id <= 4) ? race_sprite_id : 1;
            for (int d = 0; d < 4; d++) {
                equip.helmet_offset_y[d] = helmet->offset_y[race][d];
                equip.helmet_offset_x[d] = helmet->offset_x[race][d];
            }
        }
    }
    return equip;
}

void WorldRenderer::render_player_like(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
                                        Direction dir, bool moving) {
    uint8_t item_weapon, item_armor, item_helmet, item_shield;
    if (e.entity_id == state.my_entity_id) {
        item_weapon = (state.eq_weapon != 0xFF && state.eq_weapon < SnapshotDTO::INVENTORY_SIZE)
                          ? state.inventory[state.eq_weapon] : 0;
        item_armor  = (state.eq_armor  != 0xFF && state.eq_armor  < SnapshotDTO::INVENTORY_SIZE)
                          ? state.inventory[state.eq_armor]  : 0;
        item_helmet = (state.eq_helmet != 0xFF && state.eq_helmet < SnapshotDTO::INVENTORY_SIZE)
                          ? state.inventory[state.eq_helmet] : 0;
        item_shield = (state.eq_shield != 0xFF && state.eq_shield < SnapshotDTO::INVENTORY_SIZE)
                          ? state.inventory[state.eq_shield] : 0;
    } else {
        item_weapon = e.equipped_weapon;
        item_armor  = e.equipped_armor;
        item_helmet = e.equipped_helmet;
        item_shield = e.equipped_shield;
    }

    EquipVisual equip = build_equip_visual(item_weapon, item_armor, item_helmet, item_shield, e.sprite_id);

    const SpriteEntry& sprite = _sprite_config.get(e.sprite_id);
    SpriteBounds bounds = _anim.render(_renderer, _assets,
                 sprite.body_path, sprite.head_path,
                 e.sprite_id, dir,
                 screen_x, screen_y,
                 state.current_tick, moving,
                 &equip, e.is_ghost != 0);

    if (e.entity_id != state.my_entity_id)
        render_entity_healthbar(e, bounds, /*is_service_npc=*/false, /*name_label_offset_y=*/0);
}

void WorldRenderer::render_entities(WorldState& state, const PlayerState& player) {
    std::sort(state.entities.begin(), state.entities.end(),
        [](const EntityDTO& a, const EntityDTO& b) { return a.pos_y < b.pos_y; });

    for (const auto& e : state.entities) {
        int screen_x = _camera.world_to_screen_x(state.entity_pixel_x(e));
        int screen_y = _camera.world_to_screen_y(state.entity_pixel_y(e));

        if (e.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR)) {
            render_floor_item(e, screen_x, screen_y);
            continue;
        }

        Direction dir = static_cast<Direction>(e.direction);
        bool moving;
        if (e.entity_id == state.my_entity_id) {
            screen_x = _camera.world_to_screen_x(player.pixel_x());
            screen_y = _camera.world_to_screen_y(player.pixel_y());
            moving = player.is_moving();
        } else {
            auto it = state.entity_motion.find(e.entity_id);
            moving = (it != state.entity_motion.end() && it->second.progress < 1.0f);
        }

        if (e.entity_type == static_cast<uint8_t>(EntityType::NPC)) {
            render_npc(e, state, screen_x, screen_y, dir, moving);
            continue;
        }

        render_player_like(e, state, screen_x, screen_y, dir, moving);
    }
}

void WorldRenderer::render_entity_healthbar(const EntityDTO& entity, const SpriteBounds& bounds,
                                             bool is_service_npc, int name_label_offset_y) {
    if (entity.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR)) return;

    const int margin = 6;  // separación entre overlay y punta del sprite
    int text_anchor_y = bounds.top_y - margin;

    if (!is_service_npc) {
        const int bar_w = std::clamp(bounds.width, 24, 50);
        const int bar_h = 4;

        int bar_x = bounds.center_x - bar_w / 2;
        int bar_y = bounds.top_y - bar_h - margin;
        text_anchor_y = bar_y;

        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer.Get(), 30, 30, 30, 220);
        SDL_Rect bg_rect{ bar_x, bar_y, bar_w, bar_h };
        SDL_RenderFillRect(_renderer.Get(), &bg_rect);

        int hp_bar_w = static_cast<int>(bar_w * entity.hp_pct / 100.0f);
        if (hp_bar_w > 0) {
            SDL_Color hp_color;
            if (entity.hp_pct > 60) {
                hp_color = { 100, 220, 100, 255 };
            } else if (entity.hp_pct > 30) {
                hp_color = { 255, 220, 50, 255 };
            } else {
                hp_color = { 220, 80, 80, 255 };
            }
            SDL_SetRenderDrawColor(_renderer.Get(), hp_color.r, hp_color.g, hp_color.b, hp_color.a);
            SDL_Rect hp_rect{ bar_x, bar_y, hp_bar_w, bar_h };
            SDL_RenderFillRect(_renderer.Get(), &hp_rect);
        }

        SDL_SetRenderDrawColor(_renderer.Get(), 200, 200, 200, 255);
        SDL_RenderDrawRect(_renderer.Get(), &bg_rect);
    }

    const bool show_name = ((entity.entity_type == static_cast<uint8_t>(EntityType::PLAYER)) || is_service_npc)
                           && !entity.username.empty();
    if (!show_name) return;

    if (!_small_font) {
        const auto& fonts = ClientConfig::instance().fonts;
        _small_font = TTF_OpenFont(fonts.chat_font_path.c_str(), fonts.small_font_size);
    }
    if (!_small_font) return;

    SDL_Color name_color = { 200, 220, 255, 255 };
    SDL_Surface* surf = TTF_RenderUTF8_Blended(_small_font, entity.username.c_str(), name_color);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer.Get(), surf);
    if (tex) {
        int name_y = is_service_npc ? bounds.top_y + name_label_offset_y : text_anchor_y - surf->h - 2;
        int name_x = bounds.center_x - surf->w / 2;
        SDL_Rect dst{ name_x, name_y, surf->w, surf->h };
        SDL_RenderCopy(_renderer.Get(), tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void WorldRenderer::render_spells(WorldState& state) {
    int ticks_per_frame = ClientConfig::instance().rendering.spell_ticks_per_frame;

    state.spell_effects.erase(
        std::remove_if(state.spell_effects.begin(), state.spell_effects.end(),
            [&](const SpellEffect& fx) {
                int total = static_cast<int>(fx.frame_indices.size());
                uint32_t elapsed = state.current_tick - fx.start_tick;
                return static_cast<int>(elapsed / ticks_per_frame) >= total;
            }),
        state.spell_effects.end());

    for (const auto& fx : state.spell_effects) {
        int total = static_cast<int>(fx.frame_indices.size());
        if (total == 0 || fx.sheet_cols <= 0 || fx.frame_w <= 0 || fx.frame_h <= 0) continue;

        uint32_t elapsed = state.current_tick - fx.start_tick;
        int frame = static_cast<int>(elapsed / ticks_per_frame) % total;
        int sheet_frame = fx.frame_indices[frame];

        int col = sheet_frame % fx.sheet_cols;
        int row = sheet_frame / fx.sheet_cols;

        SDL2pp::Texture& tex = _assets.get(fx.path);
        SDL2pp::Rect src(col * fx.frame_w, row * fx.frame_h, fx.frame_w, fx.frame_h);

        int center_x = _camera.world_to_screen_x(static_cast<float>(fx.pos_x * ClientConfig::instance().tile_size()))
                       + ClientConfig::instance().tile_size() / 2;
        int center_y = _camera.world_to_screen_y(static_cast<float>(fx.pos_y * ClientConfig::instance().tile_size()))
                       + ClientConfig::instance().tile_size() / 2;

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

void WorldRenderer::render_projectiles(WorldState& state) {
    uint32_t duration_ticks = ClientConfig::instance().projectiles.duration_ticks;

    state.projectiles.erase(
        std::remove_if(state.projectiles.begin(), state.projectiles.end(),
            [&](const Projectile& p) {
                return (state.current_tick - p.start_tick) >= duration_ticks;
            }),
        state.projectiles.end());

    for (const auto& p : state.projectiles) {
        float t = static_cast<float>(state.current_tick - p.start_tick)
                / static_cast<float>(duration_ticks);
        t = std::clamp(t, 0.0f, 1.0f);

        float world_x = (p.from_x + (p.to_x - p.from_x) * t) * ClientConfig::instance().tile_size() + ClientConfig::instance().tile_size() / 2.0f;
        float world_y = (p.from_y + (p.to_y - p.from_y) * t) * ClientConfig::instance().tile_size() + ClientConfig::instance().tile_size() / 2.0f;

        int sx = _camera.world_to_screen_x(world_x);
        int sy = _camera.world_to_screen_y(world_y);

        SDL_SetRenderDrawBlendMode(_renderer.Get(), SDL_BLENDMODE_BLEND);
        SDL_Color color = p.is_magic ? SDL_Color{120, 180, 255, 255}
                                      : SDL_Color{230, 210, 120, 255};
        SDL_SetRenderDrawColor(_renderer.Get(), color.r, color.g, color.b, color.a);
        SDL_Rect dot{ sx - 3, sy - 3, 6, 6 };
        SDL_RenderFillRect(_renderer.Get(), &dot);
    }
}

void WorldRenderer::render_deaths(WorldState& state) {
    const auto& death_cfg = ClientConfig::instance().death_effects;
    uint32_t now = SDL_GetTicks();

    state.death_effects.erase(
        std::remove_if(state.death_effects.begin(), state.death_effects.end(),
            [&](const DeathEffect& d) {
                return (now - d.start_ms) >= death_cfg.death_duration_ms;
            }),
        state.death_effects.end());

    for (const auto& d : state.death_effects) {
        uint32_t elapsed_ms = now - d.start_ms;
        int frame = std::clamp(
            static_cast<int>(elapsed_ms / death_cfg.death_frame_ms),
            0, death_cfg.death_frames - 1);

        std::string path = death_cfg.sprite_base_path
                         + std::to_string(frame + 1) + ".png";
        int sx = _camera.tile_to_screen_x(static_cast<int>(d.pos_x));
        int sy = _camera.tile_to_screen_y(static_cast<int>(d.pos_y));
        SDL2pp::Rect dst(sx, sy, ClientConfig::instance().tile_size(), ClientConfig::instance().tile_size());
        _renderer.Copy(_assets.get(path), SDL2pp::NullOpt, dst);
    }
}
