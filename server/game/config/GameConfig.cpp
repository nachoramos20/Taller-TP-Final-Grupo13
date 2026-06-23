#include "GameConfig.h"
#include <toml++/toml.hpp>
#include <stdexcept>
#include <algorithm>

GameConfig& GameConfig::get() {
    static GameConfig instance;
    return instance;
}

void GameConfig::load(const std::string& config_dir) {
    if (_loaded) return;

    load_game_config(config_dir + "/game_config.toml");
    load_items      (config_dir + "/items.toml");
    load_npcs       (config_dir + "/npcs.toml");
    load_spells     (config_dir + "/spells.toml");

    _loaded = true;
}

void GameConfig::load_game_config(const std::string& path) {
    auto tbl = toml::parse_file(path);

    if (auto* bs = tbl["base_stats"].as_table()) {
        _formulas.base_hp = (*bs)["base_hp"].value_or<int>(100);
        _formulas.base_mp = (*bs)["base_mp"].value_or<int>(100);
    }

    if (auto* races = tbl["race"].as_table()) {
        for (auto& [key, val] : *races) {
            uint8_t id = static_cast<uint8_t>(std::stoi(std::string(key)) - 1);
            auto* rt = val.as_table();
            if (!rt) continue;
            RaceConfig rc;
            rc.name       = (*rt)["name"].value_or<std::string>("");
            rc.hp_factor  = (*rt)["hp_factor"].value_or<double>(1.0);
            rc.mp_factor  = (*rt)["mp_factor"].value_or<double>(1.0);
            rc.base_str   = (*rt)["base_str"].value_or<int>(18);
            rc.base_agi   = (*rt)["base_agi"].value_or<int>(18);
            rc.base_int   = (*rt)["base_int"].value_or<int>(18);
            rc.base_const = (*rt)["base_const"].value_or<int>(18);
            rc.hp_regen_factor = (*rt)["hp_regen_factor"].value_or<double>(1.0);
            rc.mp_regen_factor = (*rt)["mp_regen_factor"].value_or<double>(1.0);
            _races[id] = rc;
        }
    }

    if (auto* classes = tbl["class"].as_table()) {
        for (auto& [key, val] : *classes) {
            uint8_t id = static_cast<uint8_t>(std::stoi(std::string(key)) - 1);
            auto* ct = val.as_table();
            if (!ct) continue;
            ClassConfig cc;
            cc.name                 = (*ct)["name"].value_or<std::string>("");
            cc.hp_factor            = (*ct)["hp_factor"].value_or<double>(1.0);
            cc.mp_factor            = (*ct)["mp_factor"].value_or<double>(1.0);
            cc.can_use_weapon_melee = (*ct)["can_use_weapon_melee"].value_or<bool>(true);
            cc.can_use_weapon_ranged= (*ct)["can_use_weapon_ranged"].value_or<bool>(true);
            cc.can_use_armor_heavy  = (*ct)["can_use_armor_heavy"].value_or<bool>(true);
            cc.can_meditate         = (*ct)["can_meditate"].value_or<bool>(false);
            _classes[id] = cc;
        }
    }

    // Fórmulas de combate y progresión: todas viven juntas bajo [formulas]
    // en el toml (no en sub-tablas separadas por tema).
    if (auto* formulas = tbl["formulas"].as_table()) {
        _formulas.exp_base               = (*formulas)["exp_base"].value_or<double>(1000.0);
        _formulas.exp_exponent           = (*formulas)["exp_exponent"].value_or<double>(1.8);
        _formulas.pvp_min_level          = (*formulas)["pvp_min_level"].value_or<int>(12);
        _formulas.pvp_max_level_delta    = (*formulas)["pvp_max_level_delta"].value_or<int>(10);

        _formulas.crit_multiplier        = (*formulas)["crit_multiplier"].value_or<double>(2.0);
        _formulas.crit_chance            = (*formulas)["crit_chance"].value_or<double>(0.05);
        _formulas.dodge_cap              = (*formulas)["dodge_cap"].value_or<double>(0.30);
        _formulas.dodge_per_agi          = (*formulas)["dodge_per_agi"].value_or<double>(0.005);
        _formulas.attack_cooldown_melee  = (*formulas)["attack_cooldown_melee"].value_or<int>(10);
        _formulas.attack_cooldown_spell  = (*formulas)["attack_cooldown_spell"].value_or<int>(12);

        _formulas.per_damage_base_factor = (*formulas)["per_damage_base_factor"].value_or<int>(10);
        _formulas.kill_exp_rand_max      = (*formulas)["kill_exp_rand_max"].value_or<double>(0.1);

        _formulas.drop_chance_nothing    = (*formulas)["drop_chance_nothing"].value_or<double>(0.80);
        _formulas.drop_chance_gold       = (*formulas)["drop_chance_gold"].value_or<double>(0.88);
        _formulas.drop_chance_potion     = (*formulas)["drop_chance_potion"].value_or<double>(0.89);
        _formulas.drop_chance_item       = (*formulas)["drop_chance_item"].value_or<double>(0.90);
        _formulas.gold_drop_min_frac     = (*formulas)["gold_drop_min_frac"].value_or<double>(0.01);
        _formulas.gold_drop_max_frac     = (*formulas)["gold_drop_max_frac"].value_or<double>(0.20);

        _formulas.mp_regen_per_tick      = (*formulas)["mp_regen_per_tick"].value_or<int>(2);

        _formulas.respawn_x              = (*formulas)["respawn_x"].value_or<int>(40);
        _formulas.respawn_y              = (*formulas)["respawn_y"].value_or<int>(25);
        _formulas.hp_fraction            = (*formulas)["hp_fraction"].value_or<double>(0.25);

        _formulas.tick_rate_hz           = (*formulas)["tick_rate_hz"].value_or<int>(30);
    }
}

ItemKind GameConfig::parse_item_kind(const std::string& s) {
    if (s == "weapon_melee")  return ItemKind::WEAPON_MELEE;
    if (s == "weapon_ranged") return ItemKind::WEAPON_RANGED;
    if (s == "weapon_magic")  return ItemKind::WEAPON_MAGIC;
    if (s == "armor")         return ItemKind::ARMOR;
    if (s == "helmet")        return ItemKind::HELMET;
    if (s == "shield")        return ItemKind::SHIELD;
    if (s == "potion")        return ItemKind::POTION;
    if (s == "gold")          return ItemKind::GOLD;
    return ItemKind::NONE;
}

void GameConfig::load_items(const std::string& path) {
    auto tbl = toml::parse_file(path);
    auto* items = tbl["item"].as_table();
    if (!items) return;

    for (auto& [key, val] : *items) {
        uint8_t raw_id = static_cast<uint8_t>(std::stoi(std::string(key)));
        auto* it = val.as_table();
        if (!it) continue;

        ItemConfig ic;
        ic.id          = static_cast<ItemId>(raw_id);
        ic.kind        = parse_item_kind((*it)["kind"].value_or<std::string>(""));
        ic.name        = (*it)["name"].value_or<std::string>("");
        ic.min_value   = (*it)["min_value"].value_or<int>(0);
        ic.max_value   = (*it)["max_value"].value_or<int>(0);
        ic.mana_cost   = (*it)["mana_cost"].value_or<int>(0);
        ic.range_tiles = (*it)["range_tiles"].value_or<int>(0);
        ic.sprite      = (*it)["sprite"].value_or<std::string>("");
        ic.sprite_x    = (*it)["sprite_x"].value_or<int>(0);
        ic.sprite_y    = (*it)["sprite_y"].value_or<int>(0);
        ic.sprite_w    = (*it)["sprite_w"].value_or<int>(32);
        ic.sprite_h    = (*it)["sprite_h"].value_or<int>(32);

        _items[raw_id] = ic;
    }
}

void GameConfig::load_npcs(const std::string& path) {
    auto tbl = toml::parse_file(path);
    auto* npcs = tbl["npc"].as_table();
    if (!npcs) return;

    for (auto& [key, val] : *npcs) {
        uint8_t raw_id = static_cast<uint8_t>(std::stoi(std::string(key)));
        auto* nt = val.as_table();
        if (!nt) continue;

        NpcConfig nc;
        nc.id              = static_cast<NpcId>(raw_id);
        nc.name            = (*nt)["name"].value_or<std::string>("");
        nc.max_hp          = (*nt)["max_hp"].value_or<int>(1);
        nc.dmg_min         = (*nt)["dmg_min"].value_or<int>(0);
        nc.dmg_max         = (*nt)["dmg_max"].value_or<int>(0);
        nc.defense_min     = (*nt)["defense_min"].value_or<int>(0);
        nc.defense_max     = (*nt)["defense_max"].value_or<int>(0);
        nc.attack_range    = (*nt)["attack_range"].value_or<int>(1);
        nc.move_cooldown   = (*nt)["move_cooldown"].value_or<int>(20);
        nc.attack_cooldown = (*nt)["attack_cooldown"].value_or<int>(30);
        nc.exp_reward      = (*nt)["exp_reward"].value_or<int>(0);
        nc.gold_min        = (*nt)["gold_min"].value_or<int>(0);
        nc.gold_max        = (*nt)["gold_max"].value_or<int>(0);
        nc.is_service      = (*nt)["is_service"].value_or<bool>(false);
        nc.sprite          = (*nt)["sprite"].value_or<std::string>("");
        nc.sprite_w        = (*nt)["sprite_w"].value_or<int>(64);
        nc.sprite_h        = (*nt)["sprite_h"].value_or<int>(64);

        if (auto* arr = (*nt)["drop_items"].as_array()) {
            for (auto& elem : *arr) {
                if (auto v = elem.value<int64_t>()) {
                    nc.drop_items.push_back(static_cast<uint8_t>(*v));
                }
            }
        }

        _npcs[raw_id] = nc;
        _all_npc_ids.push_back(static_cast<NpcId>(raw_id));
    }
}

void GameConfig::load_spells(const std::string& path) {
    auto tbl = toml::parse_file(path);
    auto* spells = tbl["spell"].as_table();
    if (!spells) return;

    for (auto& [key, val] : *spells) {
        uint8_t raw_id = static_cast<uint8_t>(std::stoi(std::string(key)));
        auto* st = val.as_table();
        if (!st) continue;

        SpellConfig sc;
        sc.id             = static_cast<SpellId>(raw_id);
        sc.name           = (*st)["name"].value_or<std::string>("");
        sc.spell_class    = (*st)["class"].value_or<int>(1) - 1;
        sc.mana_cost      = (*st)["mana_cost"].value_or<int>(0);
        sc.dmg_multiplier = (*st)["dmg_multiplier"].value_or<double>(1.0);
        sc.flat_bonus     = (*st)["flat_bonus"].value_or<int>(0);
        sc.range          = (*st)["range"].value_or<int>(1);
        sc.sprite         = (*st)["sprite"].value_or<std::string>("");
        sc.sprite_frames  = (*st)["sprite_frames"].value_or<int>(1);
        sc.sprite_w       = (*st)["sprite_w"].value_or<int>(96);
        sc.sprite_h       = (*st)["sprite_h"].value_or<int>(96);

        _spells[raw_id] = sc;
    }
}

const RaceConfig& GameConfig::race(uint8_t race_id) const {
    auto it = _races.find(race_id);
    if (it == _races.end()) return _fallback_race;
    return it->second;
}

const ClassConfig& GameConfig::cls(uint8_t class_id) const {
    auto it = _classes.find(class_id);
    if (it == _classes.end()) return _fallback_class;
    return it->second;
}

const ItemConfig& GameConfig::item(ItemId id) const {
    auto it = _items.find(static_cast<uint8_t>(id));
    if (it == _items.end()) return _fallback_item;
    return it->second;
}

bool GameConfig::item_exists(ItemId id) const {
    return _items.count(static_cast<uint8_t>(id)) > 0;
}

// No se aplica tabla/Factory: ItemKind y EquipSlot son ambos enums
// cerrados fijados por el diseño del juego (un arma siempre va al slot de
// arma, etc.), no catálogos de datos que vayan a crecer independientes.
EquipSlot GameConfig::equip_slot_for(ItemKind kind) const {
    switch (kind) {
        case ItemKind::WEAPON_MELEE:
        case ItemKind::WEAPON_RANGED:
        case ItemKind::WEAPON_MAGIC:  return EquipSlot::WEAPON;
        case ItemKind::ARMOR:         return EquipSlot::ARMOR;
        case ItemKind::HELMET:        return EquipSlot::HELMET;
        case ItemKind::SHIELD:        return EquipSlot::SHIELD;
        default: throw std::runtime_error("Item no equipable");
    }
}

const NpcConfig& GameConfig::npc(NpcId id) const {
    auto it = _npcs.find(static_cast<uint8_t>(id));
    if (it == _npcs.end()) return _fallback_npc;
    return it->second;
}

const std::vector<NpcId>& GameConfig::all_npc_ids() const {
    return _all_npc_ids;
}

const SpellConfig& GameConfig::spell(uint8_t spell_id) const {
    auto it = _spells.find(spell_id);
    if (it == _spells.end()) return _fallback_spell;
    return it->second;
}

const CombatFormulas& GameConfig::formulas() const {
    return _formulas;
}

uint16_t GameConfig::initial_max_hp(uint8_t race_id, uint8_t class_id) const {
    const auto& r = race(race_id);
    const auto& c = cls(class_id);
    return static_cast<uint16_t>(_formulas.base_hp * r.hp_factor * c.hp_factor);
}

uint16_t GameConfig::initial_max_mp(uint8_t race_id, uint8_t class_id) const {
    const auto& r = race(race_id);
    const auto& c = cls(class_id);
    return static_cast<uint16_t>(_formulas.base_mp * r.mp_factor * c.mp_factor);
}