#include <algorithm>
#include <cstring>
#include "WorldClans.h"
#include "WorldPlayers.h"
#include "WorldChat.h"

#include <sstream>

bool WorldClans::found(uint16_t founder_id, const std::string& clan_name) {
    const PlayerData* p = players.find(founder_id);
    if (!p) return false;

    if (player_clan.count(founder_id)) {
        chat.push_message(founder_id, 0, "Ya perteneces a un clan.");
        return false;
    }
    if (p->level < 6) {
        chat.push_message(founder_id, 0, "Necesitas nivel 6 para fundar un clan.");
        return false;
    }
    if (clans_by_name.count(clan_name)) {
        chat.push_message(founder_id, 0, "Ya existe un clan con ese nombre.");
        return false;
    }

    Clan clan;
    clan.name       = clan_name;
    clan.founder_id = founder_id;
    clan.members.push_back(founder_id);
    clans_by_name[clan_name] = clan;
    player_clan[founder_id]  = clan_name;

    PlayerData* pd = players.find_mutable(founder_id);
    if (pd) {
        PlayerData::copy_username(pd->clan_name, clan_name);
        pd->is_clan_founder = true;
    }

    chat.push_message(founder_id, 0, "Fundaste el clan \"" + clan_name + "\"!");
    return true;
}

bool WorldClans::join_request(uint16_t player_id, const std::string& clan_name) {
    auto it = clans_by_name.find(clan_name);
    if (it == clans_by_name.end()) {
        chat.push_message(player_id, 0, "No existe el clan \"" + clan_name + "\".");
        return false;
    }
    Clan& clan = it->second;

    if (player_clan.count(player_id)) {
        chat.push_message(player_id, 0, "Ya perteneces a un clan.");
        return false;
    }
    if (clan.is_banned(player_id)) {
        chat.push_message(player_id, 0, "Fuiste baneado de ese clan.");
        return false;
    }
    if (clan.is_pending(player_id)) {
        chat.push_message(player_id, 0, "Ya enviaste un pedido.");
        return false;
    }
    if ((int)clan.members.size() >= Clan::MAX_MEMBERS) {
        chat.push_message(player_id, 0, "El clan está lleno.");
        return false;
    }

    clan.pending.push_back(player_id);
    chat.push_message(player_id, 0, "Pedido enviado al clan \"" + clan_name + "\".");
    const PlayerData* pp = players.find(player_id);
    chat.push_message(clan.founder_id, 0, "Nuevo pedido de ingreso al clan de " +
        std::string(pp ? pp->username : "?") + ".");
    return true;
}

std::string WorldClans::review(uint16_t founder_id) const {
    for (const auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        std::ostringstream oss;
        oss << "=== Clan \"" << name << "\" ===\nMiembros:\n";
        for (uint16_t mid : clan.members) {
            const PlayerData* mp = players.find(mid);
            oss << "  - " << (mp ? mp->username : std::to_string(mid)) << "\n";
        }
        oss << "Pedidos pendientes:\n";
        for (uint16_t pid : clan.pending) {
            const PlayerData* pp = players.find(pid);
            oss << "  - " << (pp ? pp->username : std::to_string(pid)) << "\n";
        }
        return oss.str();
    }
    return "No eres fundador de ningún clan.";
}

bool WorldClans::accept(uint16_t founder_id, const std::string& nick) {
    uint16_t target = players.find_by_name(nick);
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        if (!clan.is_pending(target)) {
            chat.push_message(founder_id, 0, nick + " no tiene pedido pendiente.");
            return false;
        }
        clan.remove_pending(target);
        clan.members.push_back(target);
        player_clan[target] = name;

        // Persistir en PlayerData
        PlayerData* pd = players.find_mutable(target);
        if (pd) {
            PlayerData::copy_username(pd->clan_name, name);
            pd->is_clan_founder = false;
        }

        chat.push_message(target, 0, "¡Fuiste aceptado en el clan \"" + name + "\"!");
        chat.push_message(founder_id, 0, nick + " fue aceptado en el clan.");
        return true;
    }
    return false;
}

bool WorldClans::reject(uint16_t founder_id, const std::string& nick) {
    uint16_t target = players.find_by_name(nick);
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        clan.remove_pending(target);
        chat.push_message(target, 0, "Tu pedido al clan \"" + name + "\" fue rechazado.");
        chat.push_message(founder_id, 0, "Rechazaste a " + nick + ".");
        return true;
    }
    return false;
}

bool WorldClans::ban(uint16_t founder_id, const std::string& nick) {
    uint16_t target = players.find_by_name(nick);
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        clan.remove_pending(target);
        clan.remove_member(target);
        clan.banned.insert(target);
        player_clan.erase(target);

        // Limpiar PlayerData
        PlayerData* pd = players.find_mutable(target);
        if (pd) {
            std::memset(pd->clan_name, 0, sizeof(pd->clan_name));
            pd->is_clan_founder = false;
        }

        chat.push_message(target, 0, "Fuiste baneado del clan \"" + name + "\".");
        chat.push_message(founder_id, 0, "Baneaste a " + nick + " del clan.");
        return true;
    }
    return false;
}

bool WorldClans::kick(uint16_t founder_id, const std::string& nick) {
    uint16_t target = players.find_by_name(nick);
    if (target == 0) return false;
    for (auto& [name, clan] : clans_by_name) {
        if (clan.founder_id != founder_id) continue;
        if (!clan.is_member(target)) {
            chat.push_message(founder_id, 0, nick + " no es miembro del clan.");
            return false;
        }
        if (target == founder_id) {
            chat.push_message(founder_id, 0, "El fundador no puede abandonar el clan.");
            return false;
        }
        clan.remove_member(target);
        player_clan.erase(target);

        PlayerData* pd = players.find_mutable(target);
        if (pd) {
            std::memset(pd->clan_name, 0, sizeof(pd->clan_name));
            pd->is_clan_founder = false;
        }

        chat.push_message(target, 0, "Fuiste expulsado del clan \"" + name + "\".");
        chat.push_message(founder_id, 0, "Expulsaste a " + nick + " del clan.");
        return true;
    }
    return false;
}

bool WorldClans::leave(uint16_t player_id) {
    auto it = player_clan.find(player_id);
    if (it == player_clan.end()) {
        chat.push_message(player_id, 0, "No perteneces a ningún clan.");
        return false;
    }
    Clan& clan = clans_by_name[it->second];
    if (clan.founder_id == player_id) {
        chat.push_message(player_id, 0, "El fundador no puede dejar el clan.");
        return false;
    }
    clan.remove_member(player_id);
    player_clan.erase(player_id);

    PlayerData* pd = players.find_mutable(player_id);
    if (pd) {
        std::memset(pd->clan_name, 0, sizeof(pd->clan_name));
        pd->is_clan_founder = false;
    }

    chat.push_message(player_id, 0, "Abandonaste el clan.");
    return true;
}

bool WorldClans::same_clan(uint16_t a, uint16_t b) const {
    auto ia = player_clan.find(a);
    auto ib = player_clan.find(b);
    if (ia == player_clan.end() || ib == player_clan.end()) return false;
    return ia->second == ib->second;
}

void WorldClans::restore_membership(uint16_t player_id, const std::string& clan_name, bool is_founder) {
    Clan& clan = clans_by_name[clan_name];
    clan.name = clan_name;
    clan.members.push_back(player_id);
    if (is_founder) {
        clan.founder_id = player_id;
    }
    player_clan[player_id] = clan_name;
}

void WorldClans::notify_login(uint16_t player_id, bool online) {
    auto it = player_clan.find(player_id);
    if (it == player_clan.end()) return;

    const Clan& clan = clans_by_name[it->second];
    const PlayerData* p = players.find(player_id);
    std::string name = p ? std::string(p->username) : std::to_string(player_id);
    std::string msg  = "[Clan] " + name + (online ? " entró al juego." : " salió del juego.");
    for (uint16_t mid : clan.members)
        if (mid != player_id)
            chat.push_message(mid, 0, msg);
}

void WorldClans::notify_attack(uint16_t attacked_id) {
    auto it = player_clan.find(attacked_id);
    if (it == player_clan.end()) return;

    const Clan& clan = clans_by_name[it->second];
    const PlayerData* p = players.find(attacked_id);
    std::string name = p ? std::string(p->username) : std::to_string(attacked_id);
    std::string msg  = "[Clan] " + name + " está siendo atacado!";
    for (uint16_t mid : clan.members)
        if (mid != attacked_id)
            chat.push_message(mid, 0, msg);
}
