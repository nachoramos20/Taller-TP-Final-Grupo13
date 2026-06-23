#include "WorldState.h"
#include <algorithm>
#include <cmath>
#include "config/ClientConfig.h"
#include "config/SpellVfxConfig.h"

// Busca el tile de agua más cercano al jugador en un radio acotado.
float WorldState::distance_to_nearest_water_tile(const PlayerState& player) const {
    static constexpr float NOT_FOUND_DIST = 999.0f;
    if (!map_loaded) return NOT_FOUND_DIST;

    const ClientConfig::Rendering& rendering = ClientConfig::instance().rendering;
    uint16_t water_floor_id = rendering.water_floor_id;
    int      search_radius  = rendering.water_search_radius_tiles;

    int map_w = map.width;
    int map_h = map.height;
    int px = player.tile_x;
    int py = player.tile_y;

    int x0 = std::max(0, px - search_radius);
    int x1 = std::min(map_w - 1, px + search_radius);
    int y0 = std::max(0, py - search_radius);
    int y1 = std::min(map_h - 1, py + search_radius);

    float best = NOT_FOUND_DIST;
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            if (map.tiles[static_cast<size_t>(y) * map_w + x].floor_id != water_floor_id) continue;
            float dx = static_cast<float>(x - px);
            float dy = static_cast<float>(y - py);
            float d = std::sqrt(dx * dx + dy * dy);
            if (d < best) best = d;
        }
    }
    return best;
}

bool WorldState::is_floor_grass(uint16_t x, uint16_t y) const {
    if (!map_loaded) return false;
    if (x >= map.width || y >= map.height) return false;

    const ClientConfig::Rendering& rendering = ClientConfig::instance().rendering;
    uint16_t floor_id = map.tiles[static_cast<size_t>(y) * map.width + x].floor_id;
    if (floor_id >= rendering.grass_floor_id_min && floor_id <= rendering.grass_floor_id_max) return true;
    if (floor_id == rendering.sand_floor_id) return true;  // arena de la costa

    // Franjas de transición pasto<->algo más (MapaBuilder): tiles mitad
    // pasto, mitad otro piso. Suenan a pasto igual, ya que es lo que más
    // pisa el pie en esa franja.
    bool franja_arena_pasto  = (floor_id >= 40 && floor_id <= 43) || floor_id == 46;
    bool franja_rocoso_pasto = (floor_id >= 73 && floor_id <= 80);  // borde ciudad
    bool franja_tierra_pasto = (floor_id >= 81 && floor_id <= 88);  // borde pueblo
    return franja_arena_pasto || franja_rocoso_pasto || franja_tierra_pasto;
}

bool WorldState::is_floor_dirt(uint16_t x, uint16_t y) const {
    if (!map_loaded) return false;
    if (x >= map.width || y >= map.height) return false;

    uint16_t floor_id = map.tiles[static_cast<size_t>(y) * map.width + x].floor_id;
    return floor_id == ClientConfig::instance().rendering.dirt_floor_id;
}

bool WorldState::is_floor_city_stone(uint16_t x, uint16_t y) const {
    if (!map_loaded) return false;
    if (x >= map.width || y >= map.height) return false;

    uint16_t floor_id = map.tiles[static_cast<size_t>(y) * map.width + x].floor_id;
    return floor_id == ClientConfig::instance().rendering.city_stone_floor_id;
}

bool is_in_forest_zone(uint16_t x, uint16_t y) {
    const ClientConfig::Rendering& r = ClientConfig::instance().rendering;
    if (static_cast<int>(x) < r.forest_x_min || static_cast<int>(x) > r.forest_x_max) return false;
    int iy = static_cast<int>(y);
    return (iy >= r.forest_y1_min && iy <= r.forest_y1_max) ||
           (iy >= r.forest_y2_min && iy <= r.forest_y2_max);
}

// Distancia del jugador al rectángulo del cementerio: 0 si está adentro,
// sino la distancia al punto más cercano del borde.
float distance_to_cemetery_zone(int x, int y) {
    const ClientConfig::Rendering& r = ClientConfig::instance().rendering;
    int nearest_x = std::clamp(x, r.cemetery_x_min, r.cemetery_x_max);
    int nearest_y = std::clamp(y, r.cemetery_y_min, r.cemetery_y_max);
    float dx = static_cast<float>(x - nearest_x);
    float dy = static_cast<float>(y - nearest_y);
    return std::sqrt(dx * dx + dy * dy);
}

bool is_in_safe_zone(uint16_t x, uint16_t y) {
    const ClientConfig::Rendering& r = ClientConfig::instance().rendering;
    int ix = static_cast<int>(x), iy = static_cast<int>(y);

    bool in_zone1 = ix >= r.safe_zone1_x_min && ix <= r.safe_zone1_x_max &&
                    iy >= r.safe_zone1_y_min && iy <= r.safe_zone1_y_max;
    bool in_zone2 = ix >= r.safe_zone2_x_min && ix <= r.safe_zone2_x_max &&
                    iy >= r.safe_zone2_y_min && iy <= r.safe_zone2_y_max;
    return in_zone1 || in_zone2;
}

uint8_t WorldState::own_weapon_item() const {
    return (eq_weapon != 0xFF && eq_weapon < SnapshotDTO::INVENTORY_SIZE)
               ? inventory[eq_weapon]
               : 0;
}

void WorldState::spawn_spell_effect(uint8_t spell_id, uint16_t pos_x, uint16_t pos_y) {
    if (!SpellVfxConfig::instance().has_spell(spell_id)) return;
    const SpellVfxConfig::SpellEffect& effect = SpellVfxConfig::instance().get_effect_info(spell_id);

    SpellEffect fx{};
    fx.spell_id      = spell_id;
    fx.pos_x         = pos_x;
    fx.pos_y         = pos_y;
    fx.start_tick    = current_tick;
    fx.sheet_cols    = effect.sheet_cols;
    fx.frame_w       = effect.frame_w;
    fx.frame_h       = effect.frame_h;
    fx.frame_indices = effect.frame_indices;
    fx.path          = effect.path;
    spell_effects.push_back(fx);
}

void WorldState::spawn_projectile(uint16_t from_x, uint16_t from_y,
                                  uint16_t to_x, uint16_t to_y, bool is_magic) {
    Projectile p{};
    p.from_x     = from_x;
    p.from_y     = from_y;
    p.to_x       = to_x;
    p.to_y       = to_y;
    p.start_tick = current_tick;
    p.is_magic   = is_magic;
    projectiles.push_back(p);
}

void WorldState::update_entity_motion(const std::vector<EntityDTO>& new_entities) {
    for (const EntityDTO& e : new_entities) {
        float new_x = static_cast<float>(e.pos_x);
        float new_y = static_cast<float>(e.pos_y);

        std::unordered_map<uint16_t, EntityMotion>::iterator it = entity_motion.find(e.entity_id);
        if (it == entity_motion.end()) {
            // Entidad nueva (recién entra en rango): aparece directo, sin
            // deslizar desde un origen arbitrario.
            entity_motion[e.entity_id] = EntityMotion{ new_x, new_y, new_x, new_y, 1.0f };
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
    for (std::unordered_map<uint16_t, EntityMotion>::iterator it = entity_motion.begin();
         it != entity_motion.end();) {
        bool found = false;
        for (const EntityDTO& e : new_entities)
            if (e.entity_id == it->first) { found = true; break; }
        it = found ? std::next(it) : entity_motion.erase(it);
    }
}

void WorldState::advance_entity_motion(float dt) {
    for (std::pair<const uint16_t, EntityMotion>& motion_entry : entity_motion) {
        EntityMotion& motion = motion_entry.second;
        if (motion.progress >= 1.0f) continue;
        motion.progress += dt / MOVE_DURATION;
        if (motion.progress > 1.0f) motion.progress = 1.0f;
    }
}

float WorldState::entity_pixel_x(const EntityDTO& e) const {
    std::unordered_map<uint16_t, EntityMotion>::const_iterator it = entity_motion.find(e.entity_id);
    if (it == entity_motion.end()) return static_cast<float>(e.pos_x * ClientConfig::instance().tile_size());
    const EntityMotion& m = it->second;
    return (m.from_x + (m.to_x - m.from_x) * m.progress) * ClientConfig::instance().tile_size();
}

float WorldState::entity_pixel_y(const EntityDTO& e) const {
    std::unordered_map<uint16_t, EntityMotion>::const_iterator it = entity_motion.find(e.entity_id);
    if (it == entity_motion.end()) return static_cast<float>(e.pos_y * ClientConfig::instance().tile_size());
    const EntityMotion& m = it->second;
    return (m.from_y + (m.to_y - m.from_y) * m.progress) * ClientConfig::instance().tile_size();
}
