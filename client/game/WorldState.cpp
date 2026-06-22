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

int manhattan_dist_to_player_tiles(const PlayerState& player, uint16_t x, uint16_t y) {
    int dx = static_cast<int>(x) - static_cast<int>(player.tile_x);
    int dy = static_cast<int>(y) - static_cast<int>(player.tile_y);
    return std::abs(dx) + std::abs(dy);
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

bool is_floor_grass(const WorldState& state, uint16_t x, uint16_t y) {
    if (!state.map_loaded) return false;
    if (x >= state.map.width || y >= state.map.height) return false;

    const auto& rendering = ClientConfig::instance().rendering;
    uint16_t floor_id = state.map.tiles[static_cast<size_t>(y) * state.map.width + x].floor_id;
    if (floor_id >= rendering.grass_floor_id_min && floor_id <= rendering.grass_floor_id_max) return true;

    // Franjas de transición pasto<->algo más (MapaBuilder): tiles mitad
    // pasto, mitad otro piso. Suenan a pasto igual, ya que es lo que más
    // pisa el pie en esa franja.
    bool franja_arena_pasto  = (floor_id >= 40 && floor_id <= 43) || floor_id == 46;
    bool franja_rocoso_pasto = (floor_id >= 73 && floor_id <= 80);  // borde ciudad
    bool franja_tierra_pasto = (floor_id >= 81 && floor_id <= 88);  // borde pueblo
    return franja_arena_pasto || franja_rocoso_pasto || franja_tierra_pasto;
}

bool is_floor_dirt(const WorldState& state, uint16_t x, uint16_t y) {
    if (!state.map_loaded) return false;
    if (x >= state.map.width || y >= state.map.height) return false;

    uint16_t floor_id = state.map.tiles[static_cast<size_t>(y) * state.map.width + x].floor_id;
    return floor_id == ClientConfig::instance().rendering.dirt_floor_id;
}

bool is_floor_city_stone(const WorldState& state, uint16_t x, uint16_t y) {
    if (!state.map_loaded) return false;
    if (x >= state.map.width || y >= state.map.height) return false;

    uint16_t floor_id = state.map.tiles[static_cast<size_t>(y) * state.map.width + x].floor_id;
    return floor_id == ClientConfig::instance().rendering.city_stone_floor_id;
}

bool is_in_forest_zone(uint16_t x, uint16_t y) {
    const auto& r = ClientConfig::instance().rendering;
    if (static_cast<int>(x) < r.forest_x_min || static_cast<int>(x) > r.forest_x_max) return false;
    int iy = static_cast<int>(y);
    return (iy >= r.forest_y1_min && iy <= r.forest_y1_max) ||
           (iy >= r.forest_y2_min && iy <= r.forest_y2_max);
}

// Distancia del jugador al rectángulo del cementerio: 0 si está adentro,
// sino la distancia al punto más cercano del borde.
float distance_to_cemetery_zone(int x, int y) {
    const auto& r = ClientConfig::instance().rendering;
    int nearest_x = std::clamp(x, r.cemetery_x_min, r.cemetery_x_max);
    int nearest_y = std::clamp(y, r.cemetery_y_min, r.cemetery_y_max);
    float dx = static_cast<float>(x - nearest_x);
    float dy = static_cast<float>(y - nearest_y);
    return std::sqrt(dx * dx + dy * dy);
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

void update_entity_motion(WorldState& state, const std::vector<EntityDTO>& new_entities) {
    for (const auto& e : new_entities) {
        float new_x = static_cast<float>(e.pos_x);
        float new_y = static_cast<float>(e.pos_y);

        auto it = state.entity_motion.find(e.entity_id);
        if (it == state.entity_motion.end()) {
            // Entidad nueva (recién entra en rango): aparece directo, sin
            // deslizar desde un origen arbitrario.
            state.entity_motion[e.entity_id] = EntityMotion{ new_x, new_y, new_x, new_y, 1.0f };
            continue;
        }

        EntityMotion& m = it->second;
        if (m.to_x == new_x && m.to_y == new_y) continue;  // mismo destino: sigue interpolando igual

        // Arranca la nueva interpolación desde donde está parado visualmente
        // ahora (no desde el destino viejo), así no hay saltos si llega un
        // snapshot a mitad de una animación.
        float cur_x = m.from_x + (m.to_x - m.from_x) * m.progress;
        float cur_y = m.from_y + (m.to_y - m.from_y) * m.progress;
        m.from_x = cur_x;
        m.from_y = cur_y;
        m.to_x   = new_x;
        m.to_y   = new_y;
        m.progress = 0.0f;
    }

    // Descartar entidades que salieron de rango/murieron.
    for (auto it = state.entity_motion.begin(); it != state.entity_motion.end();) {
        bool found = false;
        for (const auto& e : new_entities)
            if (e.entity_id == it->first) { found = true; break; }
        it = found ? std::next(it) : state.entity_motion.erase(it);
    }
}

void advance_entity_motion(WorldState& state, float dt) {
    for (auto& [id, m] : state.entity_motion) {
        if (m.progress >= 1.0f) continue;
        m.progress += dt / MOVE_DURATION;
        if (m.progress > 1.0f) m.progress = 1.0f;
    }
}

float entity_pixel_x(const WorldState& state, const EntityDTO& e) {
    auto it = state.entity_motion.find(e.entity_id);
    if (it == state.entity_motion.end()) return static_cast<float>(e.pos_x * tile_size());
    const EntityMotion& m = it->second;
    return (m.from_x + (m.to_x - m.from_x) * m.progress) * tile_size();
}

float entity_pixel_y(const WorldState& state, const EntityDTO& e) {
    auto it = state.entity_motion.find(e.entity_id);
    if (it == state.entity_motion.end()) return static_cast<float>(e.pos_y * tile_size());
    const EntityMotion& m = it->second;
    return (m.from_y + (m.to_y - m.from_y) * m.progress) * tile_size();
}
