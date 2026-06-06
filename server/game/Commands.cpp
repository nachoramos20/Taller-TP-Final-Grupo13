#include "Commands.h"
#include "Items.h"
#include "Stats.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>

// Helpers locales

static std::mt19937& local_rng() {
    static std::mt19937 r(std::random_device{}());
    return r;
}

static int rand_range(int lo, int hi) {
    if (lo >= hi) return lo;
    return std::uniform_int_distribution<int>(lo, hi)(local_rng());
}

static int manhattan(uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by) {
    return std::abs((int)ax - (int)bx) + std::abs((int)ay - (int)by);
}

static bool fair_play_ok(const PlayerData& a, const PlayerData& b) {
    if (a.level <= 12 || b.level <= 12) return false;
    return std::abs((int)a.level - (int)b.level) <= 10;
}

// Daño = Fuerza * rand(DañoArmaMin, DañoArmaMax)
static uint16_t calc_weapon_damage(const PlayerData& attacker) {
    uint8_t wpn = attacker.equipped_weapon;
    int dmin = 1, dmax = 3;
    if (wpn != 0 && Items::exists(static_cast<ItemId>(wpn))) {
        const ItemDef& def = Items::get(static_cast<ItemId>(wpn));
        dmin = def.min_value;
        dmax = def.max_value;
    }
    int raw = attacker.strength * rand_range(dmin, dmax);
    return static_cast<uint16_t>(raw);
}

// ─── Calcular defensa total del defensor ─────────────────────────────────────
static uint16_t calc_defense(const PlayerData& defender) {
    int def = 0;
    if (defender.equipped_armor != 0 && Items::exists(static_cast<ItemId>(defender.equipped_armor))) {
        const auto& a = Items::get(static_cast<ItemId>(defender.equipped_armor));
        def += rand_range(a.min_value, a.max_value);
    }
    if (defender.equipped_helmet != 0 && Items::exists(static_cast<ItemId>(defender.equipped_helmet))) {
        const auto& h = Items::get(static_cast<ItemId>(defender.equipped_helmet));
        def += rand_range(h.min_value, h.max_value);
    }
    if (defender.equipped_shield != 0 && Items::exists(static_cast<ItemId>(defender.equipped_shield))) {
        const auto& s = Items::get(static_cast<ItemId>(defender.equipped_shield));
        def += rand_range(s.min_value, s.max_value);
    }
    return static_cast<uint16_t>(def);
}

static bool try_dodge(uint16_t agility) {
    double r = std::uniform_real_distribution<double>(0.0, 1.0)(local_rng());
    return std::pow(r, agility) < 0.001;
}

static bool is_critical() {
    return std::uniform_real_distribution<double>(0.0, 1.0)(local_rng()) < 0.05;
}

// Exp = Daño * max(NivelOtro - Nivel + 10, 0)
static uint32_t exp_per_damage(uint16_t damage, uint8_t my_level, uint8_t other_level) {
    int factor = std::max((int)other_level - (int)my_level + 10, 0);
    return static_cast<uint32_t>(damage) * static_cast<uint32_t>(factor);
}

// Exp = rand(0,0.1) * VidaMaxOtro * max(NivelOtro - Nivel + 10, 0)
static uint32_t exp_on_kill(uint16_t other_max_hp, uint8_t my_level, uint8_t other_level) {
    double r = std::uniform_real_distribution<double>(0.0, 0.1)(local_rng());
    int factor = std::max((int)other_level - (int)my_level + 10, 0);
    return static_cast<uint32_t>(r * other_max_hp * factor);
}

// Oro = rand(0, 0.2) * VidaMaxNPC
static uint32_t gold_drop_npc(uint16_t npc_max_hp) {
    double r = std::uniform_real_distribution<double>(0.0, 0.2)(local_rng());
    return static_cast<uint32_t>(r * npc_max_hp);
}

// Limite = 1000 * Nivel^1.8
static void check_level_up(PlayerData& p, World& world) {
    uint8_t orig_level = p.level;
    while (true) {
        uint32_t limit = static_cast<uint32_t>(1000.0 * std::pow((double)p.level, 1.8));
        if (p.exp < limit) break;
        p.level++;
        p.max_hp = Stats::initial_max_hp(p.race, p.cls) * p.level;
        p.max_mp = Stats::initial_max_mp(p.race, p.cls) * p.level;
        p.hp = std::min(p.hp, p.max_hp);
        p.mp = std::min(p.mp, p.max_mp);
        if (p.level != orig_level)
            world.push_message(p.entity_id, 0,
                "¡Subiste al nivel " + std::to_string(p.level) + "!");
    }
}

static void npc_drop(const NpcData& npc, const NpcTemplate& tpl, World& world) {
    double r = std::uniform_real_distribution<double>(0.0, 1.0)(local_rng());
    if (r < 0.80) return;  // nada
    if (r < 0.88) {        // 0.08 → oro
        double gr = std::uniform_real_distribution<double>(0.01, 0.2)(local_rng());
        uint32_t gold = static_cast<uint32_t>(gr * tpl.max_hp);
        world.add_floor_item(static_cast<uint8_t>(ItemId::GOLD_PILE), npc.pos_x, npc.pos_y, gold);
        return;
    }
    if (r < 0.89) {        // 0.01 → poción aleatoria
        uint8_t pot = (rand_range(0, 1) == 0)
            ? static_cast<uint8_t>(ItemId::HEALTH_POTION)
            : static_cast<uint8_t>(ItemId::MANA_POTION);
        world.add_floor_item(pot, npc.pos_x, npc.pos_y, 0);
        return;
    }
    if (r < 0.90) {        // 0.01 → item de drop_table
        if (!tpl.drop_table.empty()) {
            int idx = rand_range(0, (int)tpl.drop_table.size() - 1);
            world.add_floor_item(tpl.drop_table[idx], npc.pos_x, npc.pos_y, 0);
        }
        return;
    }
}

// MoveCommand

MoveCommand::MoveCommand(uint16_t c, uint16_t x, uint16_t y)
    : client_id(c), new_x(x), new_y(y) {}

void MoveCommand::execute(World& world) {
    world.move_player(client_id, new_x, new_y);
}

// LoginCommand

LoginCommand::LoginCommand(PlayerData p) : player_data(std::move(p)) {}

void LoginCommand::execute(World& world) {
    // Hardcode de arma para testing: damos espada en slot 0 y la equipamos.
    player_data.inventory[0]    = static_cast<uint8_t>(ItemId::SWORD);
    player_data.equipped_weapon = static_cast<uint8_t>(ItemId::SWORD);
    world.add_player(player_data);
}

const char* LoginCommand::get_username() const {
    return player_data.username;
}

// LogoutCommand

LogoutCommand::LogoutCommand(uint16_t c) : client_id(c) {}

void LogoutCommand::execute(World& world) {
    world.remove_player(client_id);
}

// AttackCommand  (jugador → jugador)

AttackCommand::AttackCommand(uint16_t c, uint16_t t)
    : client_id(c), target_id(t) {}

void AttackCommand::execute(World& world) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    PlayerData* target   = world.get_player_mutable(target_id);

    if (!attacker) return;

    // Si el target no es un jugador, probamos NPC y delegamos.
    if (!target) {
        if (world.find_npc(target_id)) {
            AttackNpcCommand npc_cmd(client_id, target_id);
            npc_cmd.execute(world);
        }
        return;
    }
    if (attacker->is_ghost || target->is_ghost) return;
    if (attacker->attack_cooldown > 0) return;

    // Fair-play
    if (!fair_play_ok(*attacker, *target)) {
        world.push_message(client_id, 0, "No puedes atacar a ese jugador (fair-play).");
        return;
    }
    // Compañeros de clan no pueden atacarse
    if (world.same_clan(client_id, target_id)) {
        world.push_message(client_id, 0, "No puedes atacar a un compañero de clan.");
        return;
    }

    attacker->meditating = false;

    // Determinar si es arma a distancia o cuerpo a cuerpo
    bool ranged = false;
    if (attacker->equipped_weapon != 0 && Items::exists(static_cast<ItemId>(attacker->equipped_weapon))) {
        ranged = (Items::get(static_cast<ItemId>(attacker->equipped_weapon)).kind == ItemKind::WEAPON_RANGED);
    }
    int dist = manhattan(attacker->pos_x, attacker->pos_y, target->pos_x, target->pos_y);
    int max_dist = ranged ? 8 : 1;
    if (dist > max_dist) {
        world.push_message(client_id, 0, "Objetivo demasiado lejos.");
        return;
    }

    // Ataque crítico (no esquivable, doble daño)
    bool crit = is_critical();

    // Esquive (solo si no es crítico)
    if (!crit && try_dodge(target->agility)) {
        world.push_message(client_id, 0, "¡" + std::string(target->username) + " esquivó tu ataque!");
        world.push_message(target_id, 1, "Esquivaste el ataque de " + std::string(attacker->username) + "!");
        attacker->attack_cooldown = 10;
        return;
    }

    uint16_t damage = calc_weapon_damage(*attacker);
    if (crit) damage *= 2;

    if (!crit) {
        uint16_t def = calc_defense(*target);
        damage = (damage > def) ? damage - def : 1;
    }

    uint32_t xp_gained = exp_per_damage(damage, attacker->level, target->level);
    attacker->exp += xp_gained;
    check_level_up(*attacker, world);

    std::string crit_str = crit ? " [CRITICO]" : "";
    world.push_message(client_id, 1, "Hiciste " + std::to_string(damage) + " de daño" + crit_str + ".");
    world.push_message(target_id, 1, "Recibiste " + std::to_string(damage) + " de daño de " +
        std::string(attacker->username) + crit_str + ".");

    world.clan_notify_attack(target_id);

    if (target->hp <= damage) {
        target->hp       = 0;
        target->is_ghost = true;
        target->meditating = false;
        world.update_occupied({target->pos_x, target->pos_y}, false);
        world.drop_player_loot(*target);

        attacker->exp += exp_on_kill(target->max_hp, attacker->level, target->level);
        check_level_up(*attacker, world);

        world.push_message(target_id, 1, "Moriste! Eres un fantasma.");
        world.push_message(client_id, 1, "Mataste a " + std::string(target->username) + "!");
    } else {
        target->hp -= damage;
    }

    attacker->attack_cooldown = 10;
}

// AttackNpcCommand  (jugador → NPC)

AttackNpcCommand::AttackNpcCommand(uint16_t c, uint16_t n)
    : client_id(c), npc_id(n) {}

void AttackNpcCommand::execute(World& world) {
    PlayerData* attacker = world.get_player_mutable(client_id);
    NpcData*    npc      = world.find_npc(npc_id);

    if (!attacker || !npc) return;
    if (attacker->is_ghost) return;
    if (attacker->attack_cooldown > 0) return;

    attacker->meditating = false;

    const NpcTemplate& tpl = Npcs::tpl(npc->type);

    bool ranged = false;
    if (attacker->equipped_weapon != 0 && Items::exists(static_cast<ItemId>(attacker->equipped_weapon)))
        ranged = (Items::get(static_cast<ItemId>(attacker->equipped_weapon)).kind == ItemKind::WEAPON_RANGED);

    int dist     = manhattan(attacker->pos_x, attacker->pos_y, npc->pos_x, npc->pos_y);
    int max_dist = ranged ? 8 : 1;
    if (dist > max_dist) {
        world.push_message(client_id, 0, "El NPC está demasiado lejos.");
        return;
    }

    bool crit = is_critical();

    if (!crit && try_dodge(10)) {
        world.push_message(client_id, 1, "El NPC esquivó tu ataque!");
        attacker->attack_cooldown = 10;
        return;
    }

    uint16_t damage = calc_weapon_damage(*attacker);
    if (crit) damage *= 2;

    if (!crit) {
        int def = rand_range(tpl.defense_min, tpl.defense_max);
        damage = (damage > (uint16_t)def) ? damage - (uint16_t)def : 1;
    }

    uint32_t xp = exp_per_damage(damage, attacker->level, 1 /* nivel NPC base */);
    attacker->exp += xp;
    check_level_up(*attacker, world);

    std::string crit_str = crit ? " [CRITICO]" : "";
    world.push_message(client_id, 1,
        "Hiciste " + std::to_string(damage) + " de daño al " + tpl.name + crit_str + ".");

    if (npc->hp <= damage) {
        // NPC muerto
        uint32_t xp_kill = exp_on_kill(npc->max_hp, attacker->level, 1);
        attacker->exp += xp_kill + tpl.exp_reward;
        check_level_up(*attacker, world);

        // Oro del NPC
        uint32_t gold = gold_drop_npc(npc->max_hp);
        attacker->gold += gold;

        // Drop de items
        npc_drop(*npc, tpl, world);

        world.push_message(client_id, 1,
            "Mataste al " + tpl.name + "! +" + std::to_string(gold) + " oro.");

        // Eliminar NPC del vector (marcamos hp=0; World::tick_npcs lo eliminará)
        npc->hp = 0;
    } else {
        npc->hp -= damage;
    }

    attacker->attack_cooldown = 10;
}

// EquipCommand

EquipCommand::EquipCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void EquipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;
    if (inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t raw_id = p->inventory[inv_slot];
    if (raw_id == 0) return;

    ItemId item_id = static_cast<ItemId>(raw_id);
    if (!Items::exists(item_id)) return;

    const ItemDef& def = Items::get(item_id);
    if (def.kind == ItemKind::NONE || def.kind == ItemKind::POTION ||
        def.kind == ItemKind::GOLD) return;

    // Guerrero no puede equipar armas mágicas (staves)
    if (static_cast<Class>(p->cls) == Class::WARRIOR &&
        def.kind == ItemKind::WEAPON_RANGED && def.mana_cost > 0) {
        world.push_message(client_id, 0, "El Guerrero no puede usar magia.");
        return;
    }

    EquipSlot slot = Items::equip_slot_for(def.kind);

    // No puede tener arma física Y báculo a la vez
    if (slot == EquipSlot::WEAPON) {
        bool new_is_magic   = (def.mana_cost > 0);
        bool curr_is_magic  = false;
        if (p->equipped_weapon != 0 && Items::exists(static_cast<ItemId>(p->equipped_weapon)))
            curr_is_magic = (Items::get(static_cast<ItemId>(p->equipped_weapon)).mana_cost > 0);
        if (new_is_magic != curr_is_magic && p->equipped_weapon != 0) {
            world.push_message(client_id, 0, "No puedes tener arma y báculo equipados a la vez.");
            return;
        }
    }

    // Devolver el item actualmente equipado al inventario si hay uno
    uint8_t prev = 0;
    switch (slot) {
        case EquipSlot::WEAPON: prev = p->equipped_weapon; p->equipped_weapon = raw_id; break;
        case EquipSlot::ARMOR:  prev = p->equipped_armor;  p->equipped_armor  = raw_id; break;
        case EquipSlot::HELMET: prev = p->equipped_helmet; p->equipped_helmet = raw_id; break;
        case EquipSlot::SHIELD: prev = p->equipped_shield; p->equipped_shield = raw_id; break;
    }
    p->inventory[inv_slot] = prev; // intercambio
}

// UnequipCommand

UnequipCommand::UnequipCommand(uint16_t c, EquipSlot s) : client_id(c), slot(s) {}

void UnequipCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        world.push_message(client_id, 0, "Inventario lleno, no puedes desequipar.");
        return;
    }

    uint8_t item_id = 0;
    switch (slot) {
        case EquipSlot::WEAPON: item_id = p->equipped_weapon; p->equipped_weapon = 0; break;
        case EquipSlot::ARMOR:  item_id = p->equipped_armor;  p->equipped_armor  = 0; break;
        case EquipSlot::HELMET: item_id = p->equipped_helmet; p->equipped_helmet = 0; break;
        case EquipSlot::SHIELD: item_id = p->equipped_shield; p->equipped_shield = 0; break;
    }
    if (item_id != 0)
        p->inventory[free_slot] = item_id;
}

// DropCommand  (/tirar)

DropCommand::DropCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void DropCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t item_id = p->inventory[inv_slot];
    if (item_id == 0) return;

    p->inventory[inv_slot] = 0;
    world.add_floor_item(item_id, p->pos_x, p->pos_y, 0);
}

// PickCommand  (/tomar)

PickCommand::PickCommand(uint16_t c) : client_id(c) {}

void PickCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        world.push_message(client_id, 0, "Inventario lleno.");
        return;
    }

    uint32_t gold_out = 0;
    uint8_t  item_id  = world.pick_floor_item(p->pos_x, p->pos_y, gold_out);

    if (item_id == static_cast<uint8_t>(ItemId::GOLD_PILE) && gold_out > 0) {
        p->gold += gold_out;
        world.push_message(client_id, 0, "Recogiste " + std::to_string(gold_out) + " de oro.");
        return;
    }
    if (item_id == 0) {
        world.push_message(client_id, 0, "No hay nada aquí.");
        return;
    }
    p->inventory[free_slot] = item_id;
    if (Items::exists(static_cast<ItemId>(item_id)))
        world.push_message(client_id, 0,
            "Recogiste: " + Items::get(static_cast<ItemId>(item_id)).name);
}

// UseItemCommand  (equipar poción → se consume)

UseItemCommand::UseItemCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}

void UseItemCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost || inv_slot >= PlayerData::INVENTORY_SIZE) return;

    uint8_t raw_id = p->inventory[inv_slot];
    if (raw_id == 0) return;

    ItemId item_id = static_cast<ItemId>(raw_id);
    if (!Items::exists(item_id)) return;

    const ItemDef& def = Items::get(item_id);
    if (def.kind != ItemKind::POTION) return;

    p->meditating = false;

    if (item_id == ItemId::HEALTH_POTION) {
        p->hp = std::min(p->max_hp, static_cast<uint16_t>(p->hp + def.min_value));
        world.push_message(client_id, 0, "Usaste Pocion de Vida. HP +" + std::to_string(def.min_value));
    } else if (item_id == ItemId::MANA_POTION) {
        p->mp = std::min(p->max_mp, static_cast<uint16_t>(p->mp + def.min_value));
        world.push_message(client_id, 0, "Usaste Pocion de Mana. MP +" + std::to_string(def.min_value));
    }
    p->inventory[inv_slot] = 0;
}

// MeditateCommand  (/meditar)

MeditateCommand::MeditateCommand(uint16_t c) : client_id(c) {}

void MeditateCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    // Guerrero no puede meditar
    if (static_cast<Class>(p->cls) == Class::WARRIOR) {
        world.push_message(client_id, 0, "El Guerrero no puede meditar.");
        return;
    }

    p->meditating = !p->meditating;
    if (p->meditating)
        world.push_message(client_id, 0, "Entraste en meditación.");
    else
        world.push_message(client_id, 0, "Saliste de la meditación.");
}

// ResurrectCommand  (/resucitar)

ResurrectCommand::ResurrectCommand(uint16_t c) : client_id(c) {}

void ResurrectCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || !p->is_ghost) return;

    world.update_occupied({p->pos_x, p->pos_y}, false);
    p->pos_x      = 5;
    p->pos_y      = 5;
    p->is_ghost   = false;
    p->meditating = false;
    p->hp         = p->max_hp / 4;
    p->mp         = 0;
    world.update_occupied({p->pos_x, p->pos_y}, true);
    world.push_message(client_id, 0, "Resucitaste junto al sanador.");
}

// NpcInteractCommand  (click en NPC)

NpcInteractCommand::NpcInteractCommand(uint16_t c, uint16_t n)
    : client_id(c), npc_id(n) {}

void NpcInteractCommand::execute(World& world) {
    // La interacción con NPC especiales (sacerdote, comerciante, banquero) se
    // maneja a través de ChatCommand dirigido al NPC seleccionado.
    // Aquí simplemente informamos qué NPC fue seleccionado.
    NpcData* npc = world.find_npc(npc_id);
    if (!npc) {
        world.push_message(client_id, 0, "NPC no encontrado.");
        return;
    }
    const NpcTemplate& tpl = Npcs::tpl(npc->type);
    world.push_message(client_id, 0,
        "Seleccionaste a " + tpl.name + ". Usa /curar, /comprar, /depositar, etc.");
}

// ChatCommand  — parsea y delega

ChatCommand::ChatCommand(uint16_t c, std::string cmd)
    : client_id(c), cmd(std::move(cmd)) {}

void ChatCommand::execute(World& world) {
    // Tokenizar
    std::istringstream ss(cmd);
    std::string token;
    ss >> token;

    // Mensaje privado: @nick mensaje
    if (!token.empty() && token[0] == '@') {
        handle_private_msg(world, cmd);
        return;
    }

    // Obtener argumento del resto del string
    std::string rest;
    std::getline(ss, rest);
    if (!rest.empty() && rest[0] == ' ') rest = rest.substr(1);

    if (token == "/meditar")        { handle_meditar(world); }
    else if (token == "/resucitar") { handle_resucitar(world); }
    else if (token == "/curar")     { handle_curar(world); }
    else if (token == "/depositar") { handle_depositar(world, rest); }
    else if (token == "/retirar")   { handle_retirar(world, rest); }
    else if (token == "/listar")    { handle_listar(world); }
    else if (token == "/comprar")   { handle_comprar(world, rest); }
    else if (token == "/vender")    { handle_vender(world, rest); }
    else if (token == "/tomar")     { handle_tomar(world); }
    else if (token == "/tirar")     { handle_tirar(world, rest); }
    else if (token == "/fundar-clan")   { handle_fundar_clan(world, rest); }
    else if (token == "/unirse")        { handle_unirse(world, rest); }
    else if (token == "/revisar-clan")  { handle_revisar_clan(world); }
    else if (token == "/clan-aceptar")  { handle_clan_aceptar(world, rest); }
    else if (token == "/clan-rechazar") { handle_clan_rechazar(world, rest); }
    else if (token == "/clan-ban")      { handle_clan_ban(world, rest); }
    else if (token == "/clan-kick")     { handle_clan_kick(world, rest); }
    else if (token == "/dejar-clan")    { handle_dejar_clan(world); }
    else {
        world.push_message(client_id, 0, "Comando desconocido: " + token);
    }
}

// ─── Sub-handlers ─────────────────────────────────────────────────────────────

void ChatCommand::handle_meditar(World& world) {
    MeditateCommand(client_id).execute(world);
}

void ChatCommand::handle_resucitar(World& world) {
    ResurrectCommand(client_id).execute(world);
}

void ChatCommand::handle_curar(World& world) {
    // Curación por sacerdote: restaura HP y MP completos
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    if (p->is_ghost) {
        world.push_message(client_id, 0, "Eres un fantasma, no puedes ser curado.");
        return;
    }
    p->hp = p->max_hp;
    p->mp = p->max_mp;
    world.push_message(client_id, 0, "El sacerdote te curó completamente.");
}

void ChatCommand::handle_depositar(World& world, const std::string& args) {
    // /depositar oro <cant>
    // /depositar <objeto>
    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (first == "oro") {
        uint32_t amount = 0;
        ss >> amount;
        world.bank_deposit_gold(client_id, amount);
    } else {
        // depositar objeto por nombre (usa slot seleccionado: simplificado → primer match)
        PlayerData* p = world.get_player_mutable(client_id);
        if (!p) return;
        std::string item_name = args;
        for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
            if (p->inventory[i] == 0) continue;
            if (Items::exists(static_cast<ItemId>(p->inventory[i]))) {
                const auto& def = Items::get(static_cast<ItemId>(p->inventory[i]));
                if (def.name == item_name) {
                    world.bank_deposit_item(client_id, static_cast<uint8_t>(i));
                    return;
                }
            }
        }
        world.push_message(client_id, 0, "No encontraste ese objeto en tu inventario.");
    }
}

void ChatCommand::handle_retirar(World& world, const std::string& args) {
    // /retirar oro <cant>
    // /retirar <objeto>
    std::istringstream ss(args);
    std::string first;
    ss >> first;

    if (first == "oro") {
        uint32_t amount = 0;
        ss >> amount;
        world.bank_withdraw_gold(client_id, amount);
    } else {
        world.bank_withdraw_item(client_id, args);
    }
}

void ChatCommand::handle_listar(World& world) {
    std::string listing = world.bank_list(client_id);
    world.push_message(client_id, 0, listing.empty() ? "Banco vacío." : listing);
}

void ChatCommand::handle_comprar(World& world, const std::string& item_name) {
    // Tienda simplificada: el sacerdote vende pociones y báculos;
    // el comerciante vende armas, armaduras. Aquí implementamos la lógica
    // general buscando el item en el catálogo y cobrando un precio base.
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    // Buscar free slot
    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        world.push_message(client_id, 0, "Inventario lleno.");
        return;
    }

    // Buscar item en catálogo por nombre
    // (iteramos todos los IDs conocidos)
    static const std::vector<ItemId> all_items = {
        ItemId::SWORD, ItemId::AXE, ItemId::HAMMER,
        ItemId::SIMPLE_BOW, ItemId::COMPOUND_BOW,
        ItemId::ELVEN_FLUTE, ItemId::GEMMED_STAFF,
        ItemId::LEATHER_ARMOR, ItemId::PLATE_ARMOR,
        ItemId::HOOD, ItemId::IRON_HELMET, ItemId::MAGIC_HAT,
        ItemId::TURTLE_SHIELD, ItemId::IRON_SHIELD,
        ItemId::HEALTH_POTION, ItemId::MANA_POTION,
    };

    for (ItemId iid : all_items) {
        const ItemDef& def = Items::get(iid);
        if (def.name == item_name) {
            // Precio = max_value * 10
            uint32_t price = def.max_value * 10 + 10;
            if (p->gold < price) {
                world.push_message(client_id, 0,
                    "Oro insuficiente. Necesitas " + std::to_string(price) + " de oro.");
                return;
            }
            p->gold -= price;
            p->inventory[free_slot] = static_cast<uint8_t>(iid);
            world.push_message(client_id, 0,
                "Compraste " + item_name + " por " + std::to_string(price) + " de oro.");
            return;
        }
    }
    world.push_message(client_id, 0, "El vendedor no tiene ese artículo.");
}

void ChatCommand::handle_vender(World& world, const std::string& item_name) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) continue;
        if (!Items::exists(static_cast<ItemId>(p->inventory[i]))) continue;
        const ItemDef& def = Items::get(static_cast<ItemId>(p->inventory[i]));
        if (def.name == item_name) {
            // Precio de venta = 50% del precio de compra
            uint32_t sell_price = (def.max_value * 10 + 10) / 2;
            p->gold += sell_price;
            p->inventory[i] = 0;
            world.push_message(client_id, 0,
                "Vendiste " + item_name + " por " + std::to_string(sell_price) + " de oro.");
            return;
        }
    }
    world.push_message(client_id, 0, "No tienes ese objeto para vender.");
}

void ChatCommand::handle_tomar(World& world) {
    PickCommand(client_id).execute(world);
}

void ChatCommand::handle_tirar(World& world, const std::string& args) {
    // /tirar <nombre_item> o /tirar <slot_numero>
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;

    // Intentar como número de slot
    try {
        int slot = std::stoi(args);
        if (slot >= 0 && slot < PlayerData::INVENTORY_SIZE) {
            DropCommand(client_id, static_cast<uint8_t>(slot)).execute(world);
            return;
        }
    } catch (...) {}

    // Buscar por nombre
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) continue;
        if (Items::exists(static_cast<ItemId>(p->inventory[i]))) {
            if (Items::get(static_cast<ItemId>(p->inventory[i])).name == args) {
                DropCommand(client_id, static_cast<uint8_t>(i)).execute(world);
                return;
            }
        }
    }
    world.push_message(client_id, 0, "No encontraste ese objeto en tu inventario.");
}

void ChatCommand::handle_private_msg(World& world, const std::string& full_cmd) {
    // @nick mensaje
    std::istringstream ss(full_cmd);
    std::string target_token;
    ss >> target_token;
    if (target_token.size() < 2) return;
    std::string target_nick = target_token.substr(1);

    std::string msg;
    std::getline(ss, msg);
    if (!msg.empty() && msg[0] == ' ') msg = msg.substr(1);

    uint16_t target_id = world.find_player_by_name(target_nick);
    if (target_id == 0) {
        world.push_message(client_id, 0, "Jugador " + target_nick + " no encontrado.");
        return;
    }

    const PlayerData* sender = world.find_player(client_id);
    std::string sender_name  = sender ? std::string(sender->username) : "?";

    world.push_message(target_id, 2, "[" + sender_name + " → ti]: " + msg);
    world.push_message(client_id,  2, "[tú → " + target_nick + "]: " + msg);
}

void ChatCommand::handle_fundar_clan(World& world, const std::string& clan_name) {
    if (clan_name.empty()) {
        world.push_message(client_id, 0, "Uso: /fundar-clan <nombre>");
        return;
    }
    world.clan_found(client_id, clan_name);
}

void ChatCommand::handle_unirse(World& world, const std::string& clan_name) {
    if (clan_name.empty()) {
        world.push_message(client_id, 0, "Uso: /unirse <nombre_clan>");
        return;
    }
    world.clan_join_request(client_id, clan_name);
}

void ChatCommand::handle_revisar_clan(World& world) {
    std::string info = world.clan_review(client_id);
    world.push_message(client_id, 0, info);
}

void ChatCommand::handle_clan_aceptar(World& world, const std::string& nick) {
    world.clan_accept(client_id, nick);
}

void ChatCommand::handle_clan_rechazar(World& world, const std::string& nick) {
    world.clan_reject(client_id, nick);
}

void ChatCommand::handle_clan_ban(World& world, const std::string& nick) {
    world.clan_ban(client_id, nick);
}

void ChatCommand::handle_clan_kick(World& world, const std::string& nick) {
    world.clan_kick(client_id, nick);
}

void ChatCommand::handle_dejar_clan(World& world) {
    world.clan_leave(client_id);
}
