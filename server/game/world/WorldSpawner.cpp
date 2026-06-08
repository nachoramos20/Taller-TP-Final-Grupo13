#include "WorldSpawner.h"
#include "WorldCollision.h"

bool WorldSpawner::in_safe_zone(uint16_t x, uint16_t y) const {
    for (const auto& s : safe_zones) {
        if (x >= s.x1 && x <= s.x2 && y >= s.y1 && y <= s.y2) return true;
    }
    return false;
}

bool WorldSpawner::pick_free_cell(const SpawnZone& z,
                                   uint16_t& out_x, uint16_t& out_y) {
    std::uniform_int_distribution<int> dx(z.x1, z.x2);
    std::uniform_int_distribution<int> dy(z.y1, z.y2);
    for (int i = 0; i < 20; ++i) {
        uint16_t cx = static_cast<uint16_t>(dx(rng));
        uint16_t cy = static_cast<uint16_t>(dy(rng));
        if (!collision.in_bounds(cx, cy)) continue;
        if (collision.is_occupied(cx, cy)) continue;
        if (in_safe_zone(cx, cy)) continue;   // no spawnear en zona segura
        out_x = cx; out_y = cy;
        return true;
    }
    return false;
}

std::vector<WorldSpawner::PendingSpawn>
WorldSpawner::tick(uint16_t total_alive) {
    std::vector<PendingSpawn> out;

    for (uint8_t i = 0; i < zones.size(); ++i) {
        auto& z = zones[i];
        if (z.cooldown > 0) { z.cooldown--; continue; }
        if (z.allowed_types.empty()) continue;
        if (total_alive >= global_cap) break;
        if (z.alive_count >= z.max_alive) {
            z.cooldown = z.spawn_every_ticks;
            continue;
        }

        uint16_t x = 0, y = 0;
        if (!pick_free_cell(z, x, y)) {
            z.cooldown = z.spawn_every_ticks / 2;  // reintenta antes
            continue;
        }

        std::uniform_int_distribution<int> td(0, (int)z.allowed_types.size() - 1);
        NpcId type = z.allowed_types[td(rng)];

        out.push_back({i, type, x, y});
        z.alive_count++;
        total_alive++;
        z.cooldown = z.spawn_every_ticks;
    }
    return out;
}
