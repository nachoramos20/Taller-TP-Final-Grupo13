#include "Npc.h"
#include "GameConfig.h"
#include <stdexcept>

namespace Npcs {

// Convierte NpcConfig → NpcTemplate on-demand con cache estático.
const NpcTemplate& tpl(NpcId id) {
    static std::unordered_map<uint8_t, NpcTemplate> cache;
    uint8_t raw = static_cast<uint8_t>(id);

    auto it = cache.find(raw);
    if (it != cache.end()) return it->second;

    const NpcConfig& nc = GameConfig::get().npc(id);

    NpcTemplate t;
    t.id              = nc.id;
    t.name            = nc.name;
    t.max_hp          = nc.max_hp;
    t.dmg_min         = nc.dmg_min;
    t.dmg_max         = nc.dmg_max;
    t.defense_min     = nc.defense_min;
    t.defense_max     = nc.defense_max;
    t.attack_range    = nc.attack_range;
    t.move_cooldown   = nc.move_cooldown;
    t.attack_cooldown = nc.attack_cooldown;
    t.exp_reward      = nc.exp_reward;
    t.gold_min        = nc.gold_min;
    t.gold_max        = nc.gold_max;
    t.drop_table      = nc.drop_items;
    t.is_service      = nc.is_service;

    cache[raw] = t;
    return cache[raw];
}

const std::vector<NpcId>& all_ids() {
    return GameConfig::get().all_npc_ids();
}

} // namespace Npcs
