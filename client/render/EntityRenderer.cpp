#include "EntityRenderer.h"
#include <algorithm>
#include "../config/ClientConfig.h"
#include "../config/ItemVisualConfig.h"
#include "../config/NpcVisualConfig.h"

EntityRenderer::EntityRenderer(SDL2pp::Renderer& renderer, Camera& camera, AssetManager& assets)
    : _renderer(renderer), _camera(camera), _assets(assets),
      _sprite_config("config/sprites.toml") {
    _anim.load();
}

void EntityRenderer::render(WorldState& state, const PlayerState& player) {
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

void EntityRenderer::render_floor_item(const EntityDTO& e, int screen_x, int screen_y) {
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

void EntityRenderer::render_npc(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
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

EquipVisual EntityRenderer::build_equip_visual(uint8_t item_weapon, uint8_t item_armor, uint8_t item_helmet,
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

void EntityRenderer::render_player_like(const EntityDTO& e, const WorldState& state, int screen_x, int screen_y,
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

void EntityRenderer::render_entity_healthbar(const EntityDTO& entity, const SpriteBounds& bounds,
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
