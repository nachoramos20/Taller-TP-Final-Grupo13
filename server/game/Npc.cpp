#include "Npc.h"
#include <unordered_map>
#include <stdexcept>

static const std::unordered_map<uint8_t, NpcTemplate>& tpls() {
    static const std::unordered_map<uint8_t, NpcTemplate> t = {
        {(uint8_t)NpcId::GOBLIN,   {NpcId::GOBLIN,   "Goblin",   40,  3,  8,  0, 2, 1, 20, 30,  50,  3, 12, {(uint8_t)ItemId::HOOD, (uint8_t)ItemId::SWORD}}},
        {(uint8_t)NpcId::SKELETON, {NpcId::SKELETON, "Esqueleto",60,  5, 11,  1, 3, 1, 18, 28,  80,  5, 20, {(uint8_t)ItemId::IRON_HELMET, (uint8_t)ItemId::AXE}}},
        {(uint8_t)NpcId::ZOMBIE,   {NpcId::ZOMBIE,   "Zombie",   90,  6, 12,  1, 3, 1, 30, 32, 100,  6, 25, {(uint8_t)ItemId::LEATHER_ARMOR}}},
        {(uint8_t)NpcId::SPIDER,   {NpcId::SPIDER,   "Arana",    50,  4, 10,  0, 2, 1, 12, 24,  70,  2,  8, {(uint8_t)ItemId::HEALTH_POTION}}},
        {(uint8_t)NpcId::ORC,      {NpcId::ORC,      "Orco",    140,  9, 18,  2, 5, 1, 22, 30, 180, 10, 40, {(uint8_t)ItemId::IRON_SHIELD, (uint8_t)ItemId::HAMMER}}},
        {(uint8_t)NpcId::GOLEM,    {NpcId::GOLEM,    "Golem",   250, 12, 24,  4, 8, 1, 26, 36, 400, 25, 80, {(uint8_t)ItemId::PLATE_ARMOR, (uint8_t)ItemId::IRON_SHIELD}}},
        // NPCs de servicio, no tienen drops ni daño real
        {(uint8_t)NpcId::MERCHANT, {NpcId::MERCHANT, "Mercader",  9999, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, true}},
        {(uint8_t)NpcId::BANKER,   {NpcId::BANKER,   "Banquero",  9999, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, true}},
        {(uint8_t)NpcId::PRIEST,   {NpcId::PRIEST,   "Sacerdote", 9999, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, true}},
    };
    return t;
}

namespace Npcs {

const NpcTemplate& tpl(NpcId id) {
    auto it = tpls().find(static_cast<uint8_t>(id));
    if (it == tpls().end()) throw std::runtime_error("NpcId desconocido");
    return it->second;
}

const std::vector<NpcId>& all_ids() {
    static const std::vector<NpcId> v = {
        NpcId::GOBLIN, NpcId::SKELETON, NpcId::ZOMBIE,
        NpcId::SPIDER, NpcId::ORC, NpcId::GOLEM,
        NpcId::MERCHANT, NpcId::BANKER, NpcId::PRIEST,
    };
    return v;
}

}
