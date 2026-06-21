#include "WorldState.h"
#include <algorithm>
#include <cmath>
#include "../config/ClientConfig.h"
#include "../config/SpellVfxConfig.h"

float dist_to_player_tiles(const PlayerState& player, uint16_t x, uint16_t y) {
    float dx = static_cast<float>(x) - static_cast<float>(player.tile_x);
    float dy = static_cast<float>(y) - static_cast<float>(player.tile_y);
    return std::sqrt(dx * dx + dy * dy);
}

// Busca el tile de agua más cercano al jugador en un radio acotado.
float distance_to_nearest_water_tile(const WorldState& state, const PlayerState& player) {
    static constexpr float NOT_FOUND_DIST = 999.0f;
    if (!state.map_loaded) return NOT_FOUND_DIST;

    const auto& rendering = ClientConfig::instance().rendering;
    uint16_t water_floor_id = rendering.water_floor_id;
    int      search_radius  = rendering.water_search_radius_tiles;

    int map_w = state.map.width;
    int map_h = state.map.height;
    int px = player.tile_x;
    int py = player.tile_y;

    int x0 = std::max(0, px - search_radius);
    int x1 = std::min(map_w - 1, px + search_radius);
    int y0 = std::max(0, py - search_radius);
    int y1 = std::min(map_h - 1, py + search_radius);

    float best = NOT_FOUND_DIST;
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            if (state.map.tiles[static_cast<size_t>(y) * map_w + x].floor_id != water_floor_id) continue;
            float dx = static_cast<float>(x - px);
            float dy = static_cast<float>(y - py);
            float d = std::sqrt(dx * dx + dy * dy);
            if (d < best) best = d;
        }
    }
    return best;
}

uint8_t own_weapon_item(const WorldState& state) {
    return (state.eq_weapon != 0xFF && state.eq_weapon < SnapshotDTO::INVENTORY_SIZE)
               ? state.inventory[state.eq_weapon]
               : 0;
}

void spawn_spell_effect(WorldState& state, uint8_t spell_id, uint16_t pos_x, uint16_t pos_y) {
    if (!SpellVfxConfig::instance().has_spell(spell_id)) return;
    const auto& effect = SpellVfxConfig::instance().get_effect_info(spell_id);

    SpellEffect fx{};
    fx.spell_id      = spell_id;
    fx.pos_x         = pos_x;
    fx.pos_y         = pos_y;
    fx.start_tick    = state.current_tick;
    fx.sheet_cols    = effect.sheet_cols;
    fx.frame_w       = effect.frame_w;
    fx.frame_h       = effect.frame_h;
    fx.frame_indices = effect.frame_indices;
    fx.path          = effect.path;
    state.spell_effects.push_back(fx);
}

void spawn_projectile(WorldState& state, uint16_t from_x, uint16_t from_y,
                       uint16_t to_x, uint16_t to_y, bool is_magic) {
    Projectile p{};
    p.from_x     = from_x;
    p.from_y     = from_y;
    p.to_x       = to_x;
    p.to_y       = to_y;
    p.start_tick = state.current_tick;
    p.is_magic   = is_magic;
    state.projectiles.push_back(p);
}
