#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../common/protocol/protocol.h"

struct RaceConfig {
    std::string name;
    float hp_factor = 1.0f;
    float mp_factor = 1.0f;
    uint16_t base_str = 18;
    uint16_t base_agi = 18;
    uint16_t base_int = 18;
    uint16_t base_const = 18;

    // Factores de regeneración natural de HP/MP por intervalo de regen (no
    // confundir con hp_factor/mp_factor, que escalan el HP/MP máximo).
    float hp_regen_factor = 1.0f;
    float mp_regen_factor = 1.0f;
};

enum class ItemKind : uint8_t {
    NONE,
    WEAPON_MELEE,
    WEAPON_RANGED,
    WEAPON_MAGIC,  // armas mágicas: consumen mana en ataque básico y habilitan hechizos
    ARMOR,
    HELMET,
    SHIELD,
    POTION,
    GOLD
};

struct ClassConfig {
    std::string name;
    float hp_factor = 1.0f;
    float mp_factor = 1.0f;
    bool can_use_weapon_melee = true;
    bool can_use_weapon_ranged = true;
    bool can_use_armor_heavy = true;
    bool can_meditate = false;
};

struct ItemConfig {
    ItemId id;
    ItemKind kind = ItemKind::NONE;
    std::string name;
    uint16_t min_value = 0;
    uint16_t max_value = 0;
    uint16_t mana_cost = 0;
    uint16_t range_tiles = 0;
    // Sprite para el cliente
    std::string sprite;
    int sprite_x = 0;
    int sprite_y = 0;
    int sprite_w = 32;
    int sprite_h = 32;
};

struct NpcConfig {
    NpcId id;
    std::string name;
    uint16_t max_hp = 1;
    uint16_t dmg_min = 0;
    uint16_t dmg_max = 0;
    uint16_t defense_min = 0;
    uint16_t defense_max = 0;
    uint16_t attack_range = 1;
    uint16_t move_cooldown = 20;
    uint16_t attack_cooldown = 30;
    uint32_t exp_reward = 0;
    uint32_t gold_min = 0;
    uint32_t gold_max = 0;
    bool is_service = false;
    std::string sprite;
    int sprite_w = 64;
    int sprite_h = 64;
    std::vector<uint8_t> drop_items;
};

struct SpellConfig {
    SpellId id;
    std::string name;
    uint8_t spell_class = 0;  // valor de Class enum
    uint16_t mana_cost = 0;
    float dmg_multiplier = 1.0f;
    uint16_t flat_bonus = 0;
    int range = 1;
    // Sprite de animación
    std::string sprite;
    int sprite_frames = 1;
    int sprite_w = 96;
    int sprite_h = 96;
};

// Parámetros de balance leídos de game_config.toml (fórmulas).
struct CombatFormulas {
    uint16_t base_hp = 100;
    uint16_t base_mp = 100;

    double exp_base = 1000.0;
    double exp_exponent = 1.8;
    uint8_t pvp_min_level = 12;
    uint8_t pvp_max_level_delta = 10;

    double crit_multiplier = 2.0;
    double crit_chance = 0.05;
    double dodge_cap = 0.30;
    double dodge_per_agi = 0.005;
    int attack_cooldown_melee = 10;
    int attack_cooldown_spell = 12;

    int per_damage_base_factor = 10;
    double kill_exp_rand_max = 0.1;

    // Umbrales acumulados de un solo roll [0,1), no probabilidades
    // independientes: drop_chance_gold=0.88 da un 8% de oro (0.80 a 0.88).
    double drop_chance_nothing = 0.80;
    double drop_chance_gold = 0.88;
    double drop_chance_potion = 0.89;
    double drop_chance_item = 0.90;
    double gold_drop_min_frac = 0.01;
    double gold_drop_max_frac = 0.20;

    uint16_t mp_regen_per_tick = 2;

    uint16_t respawn_x = 40;
    uint16_t respawn_y = 25;
    double hp_fraction = 0.25;

    int tick_rate_hz = 30;
};

// Singleton que carga razas/clases/items/NPCs/hechizos/fórmulas desde los
// .toml de config/ al iniciar el servidor y los expone de solo lectura.
class GameConfig {
public:
    // Acceso al singleton. Llama a load() antes de get() la primera vez.
    static GameConfig& get();

    // Carga todos los archivos de configuración desde el directorio dado.
    // Lanza std::runtime_error si algún archivo no existe o tiene errores.
    void load(const std::string& config_dir = "config");

    const RaceConfig& race(uint8_t race_id) const;
    const ClassConfig& cls(uint8_t class_id) const;

    const ItemConfig& item(ItemId id) const;
    bool item_exists(ItemId id) const;
    EquipSlot equip_slot_for(ItemKind kind) const;

    const NpcConfig& npc(NpcId id) const;
    const std::vector<NpcId>& all_npc_ids() const;

    const SpellConfig& spell(uint8_t spell_id) const;

    const CombatFormulas& formulas() const;

    // Reemplazan a la vieja clase Stats::, ahora derivados de raza+clase.
    uint16_t initial_max_hp(uint8_t race_id, uint8_t class_id) const;
    uint16_t initial_max_mp(uint8_t race_id, uint8_t class_id) const;

private:
    GameConfig() = default;

    void load_game_config(const std::string& path);
    void load_items(const std::string& path);
    void load_npcs(const std::string& path);
    void load_spells(const std::string& path);

    static ItemKind parse_item_kind(const std::string& s);

    std::unordered_map<uint8_t, RaceConfig> _races;
    std::unordered_map<uint8_t, ClassConfig> _classes;
    std::unordered_map<uint8_t, ItemConfig> _items;
    std::unordered_map<uint8_t, NpcConfig> _npcs;
    std::unordered_map<uint8_t, SpellConfig> _spells;
    std::vector<NpcId> _all_npc_ids;
    CombatFormulas _formulas;

    bool _loaded = false;

    // Devueltos por referencia cuando el id pedido no existe, para que los
    // getters de arriba nunca tengan que devolver una referencia colgante.
    RaceConfig _fallback_race;
    ClassConfig _fallback_class;
    ItemConfig _fallback_item;
    NpcConfig _fallback_npc;
    SpellConfig _fallback_spell;
};
