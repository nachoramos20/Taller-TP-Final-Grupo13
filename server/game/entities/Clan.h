#ifndef CLAN_H
#define CLAN_H

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

// Un clan y su membresía. Vive en WorldClans, indexado por nombre.
struct Clan {
    std::string name;
    uint16_t founder_id = 0;
    std::vector<uint16_t> members;  // incluye al fundador
    std::vector<uint16_t> pending;  // pedidos de ingreso sin resolver
    std::unordered_set<uint16_t> banned;

    static constexpr int MAX_MEMBERS = 16;

    bool is_member(uint16_t id) const {
        return std::find(members.begin(), members.end(), id) != members.end();
    }

    bool is_pending(uint16_t id) const {
        return std::find(pending.begin(), pending.end(), id) != pending.end();
    }

    bool is_banned(uint16_t id) const { return banned.count(id) > 0; }
    void remove_member(uint16_t id) {
        members.erase(std::remove(members.begin(), members.end(), id), members.end());
    }
    void remove_pending(uint16_t id) {
        pending.erase(std::remove(pending.begin(), pending.end(), id), pending.end());
    }
};

#endif  // CLAN_H
