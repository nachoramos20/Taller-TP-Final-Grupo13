#include "PersistenceMonitor.h"
#include "Stats.h"
#include "Items.h"
#include "GameConfig.h"

#include <fstream>

PersistenceMonitor::PersistenceMonitor(Queue<PlayerData>& save_queue): save_queue(save_queue) {
    std::ifstream index_file(PLAYERS_INDEX_FILENAME, std::ios::in | std::ios::binary);
    if (!index_file.is_open()) {
        return;
    }

    IndexEntry entry{};
    while (index_file.read(reinterpret_cast<char*>(&entry), sizeof(entry))) {
        player_offsets_map[std::string(entry.username)] = entry.offset;
    }

    index_file.close();
}

static void give_item(PlayerData& p, uint8_t item_id,
                      uint8_t& equip_slot_ref, bool equip) {
    // Buscar primer hueco libre en el inventario
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; i++) {
        if (p.inventory[i] == 0) {
            p.inventory[i] = item_id;
            if (equip) equip_slot_ref = i;
            return;
        }
    }
}

static void apply_initial_equipment(PlayerData& p) {
    using I = ItemId;
    const auto cls  = static_cast<Class>(p.cls);

    // Arma inicial: siempre la de menor tier dentro de la categoría de la clase.
    // Las versiones "epicas"/"legendarias" quedan como drops/compras, no como punto de partida.
    switch (cls) {
        case Class::WARRIOR:
            give_item(p, static_cast<uint8_t>(I::SWORD),       p.equipped_weapon, true);
            break;
        case Class::PALADIN:
            give_item(p, static_cast<uint8_t>(I::SIMPLE_BOW),   p.equipped_weapon, true);
            break;
        case Class::CLERIC:
            give_item(p, static_cast<uint8_t>(I::ELVEN_FLUTE), p.equipped_weapon, true);
            break;
        case Class::MAGE:
            give_item(p, static_cast<uint8_t>(I::ASH_STICK),  p.equipped_weapon, true);
            break;
    }

    // Armadura
    switch (cls) {
        case Class::WARRIOR:
            give_item(p, static_cast<uint8_t>(I::WARRIOR_EPIC_ARMOR), p.equipped_armor, true);
            break;
        case Class::PALADIN:
            give_item(p, static_cast<uint8_t>(I::MAGE_ROYAL_ARMOR),  p.equipped_armor, true);
            break;
        case Class::CLERIC:
            give_item(p, static_cast<uint8_t>(I::CLERIC_BLACK_ARMOR),p.equipped_armor, true);
            break;
        case Class::MAGE:
            give_item(p, static_cast<uint8_t>(I::LEATHER_ARMOR),p.equipped_armor, true);
            break;
    }

    // Casco
    switch (cls) {
        case Class::WARRIOR:
        case Class::PALADIN:
            give_item(p, static_cast<uint8_t>(I::IRON_HELMET), p.equipped_helmet, true);
            break;
        case Class::CLERIC:
            give_item(p, static_cast<uint8_t>(I::HOOD),        p.equipped_helmet, true);
            break;
        case Class::MAGE:
            give_item(p, static_cast<uint8_t>(I::MAGIC_HAT),   p.equipped_helmet, true);
            break;
    }

    // Escudo (solo Warrior y Paladin)
    if (cls == Class::WARRIOR || cls == Class::PALADIN) {
        give_item(p, static_cast<uint8_t>(I::IRON_SHIELD), p.equipped_shield, true);
    }

    // Pociones iniciales (una de vida para todos)
    give_item(p, static_cast<uint8_t>(I::HEALTH_POTION), p.equipped_weapon /*dummy, no equip*/, false);
    if (cls == Class::MAGE || cls == Class::CLERIC) {
        give_item(p, static_cast<uint8_t>(I::MANA_POTION), p.equipped_weapon /*dummy*/, false);
    }

    // El Clérigo arranca con la flauta élfica equipada, que cura en vez de
    // atacar: sin un báculo de respaldo en el inventario no tendría forma
    // de hacer daño a distancia hasta comprar o encontrar otro.
    if (cls == Class::CLERIC) {
        give_item(p, static_cast<uint8_t>(I::NUDOSO_STAFF), p.equipped_weapon /*dummy, no equip*/, false);
    }
}

PlayerData PersistenceMonitor::
make_initial_player(const std::string& username, uint8_t race, uint8_t cls) {
    PlayerData data{};
    PlayerData::copy_username(data.username, username);
    data.entity_id = 0;
    data.race = race;
    data.cls  = cls;
    data.pos_x = GameConfig::get().formulas().respawn_x;
    data.pos_y = GameConfig::get().formulas().respawn_y;
    data.direction = 0;
    data.exp   = 0;
    data.level = 1;
    data.gold  = 0;
    data.is_ghost   = false;
    data.meditating = false;

    // Stats según raza y clase
    auto rf = Stats::race_of(race);

    data.strength     = rf.base_str;
    data.agility      = rf.base_agi;
    data.intelligence = rf.base_int;
    data.constitution = rf.base_const;

    data.max_hp = Stats::initial_max_hp(race, cls);
    data.hp     = data.max_hp;

    // Si la clase no puede meditar (Guerrero), no tiene maná
    if (!GameConfig::get().cls(cls).can_meditate) {
        data.max_mp = 0;
        data.mp     = 0;
    } else {
        data.max_mp = Stats::initial_max_mp(race, cls);
        data.mp     = data.max_mp;
    }

    // Inventario y equipo según clase
    apply_initial_equipment(data);

    return data;
}

bool PersistenceMonitor::login(const std::string& target_username,
                               PlayerData& player_data, uint16_t entity_id) {
    std::lock_guard<std::mutex> lock(mtx);

    if (target_username.size() > PlayerData::USERNAME_MAX_LENGTH)
        return false;

    auto it = player_offsets_map.find(target_username);
    if (it == player_offsets_map.end())
        return false;

    std::ifstream data_file(PLAYERS_DATA_FILENAME, std::ios::in | std::ios::binary);
    if (!data_file.is_open())
        return false;

    data_file.seekg(static_cast<std::streamoff>(it->second));
    if (!data_file.good())
        return false;

    data_file.read(reinterpret_cast<char*>(&player_data), sizeof(PlayerData));
    if (!data_file)
        return false;

    // Garantizar invariante del guerrero en cuentas antiguas
    if (static_cast<Class>(player_data.cls) == Class::WARRIOR) {
        player_data.mp     = 0;
        player_data.max_mp = 0;
    }

    player_data.entity_id = entity_id;
    return true;
}

bool PersistenceMonitor::register_user(const std::string& new_username,
                                       uint8_t race, uint8_t cls,
                                       PlayerData& player_data, uint16_t entity_id) {
    std::lock_guard<std::mutex> lock(mtx);

    if (new_username.size() > PlayerData::USERNAME_MAX_LENGTH)
        return false;

    if (player_offsets_map.find(new_username) != player_offsets_map.end())
        return false;

    PlayerData new_player = make_initial_player(new_username, race, cls);
    new_player.entity_id = entity_id;
    player_data = new_player;

    std::ofstream data_file(PLAYERS_DATA_FILENAME, std::ios::app | std::ios::binary);
    std::ofstream index_file(PLAYERS_INDEX_FILENAME, std::ios::app | std::ios::binary);
    if (!data_file.is_open() || !index_file.is_open())
        return false;

    const uint64_t new_offset = static_cast<uint64_t>(data_file.tellp());
    data_file.write(reinterpret_cast<const char*>(&player_data), sizeof(PlayerData));
    if (!data_file)
        return false;

    IndexEntry index_entry{};
    PlayerData::copy_username(index_entry.username, new_username);
    index_entry.offset = new_offset;
    index_file.write(reinterpret_cast<const char*>(&index_entry), sizeof(IndexEntry));
    if (!index_file)
        return false;

    player_offsets_map[new_username] = new_offset;
    return true;
}

void PersistenceMonitor::save_player(const PlayerData& player_data_to_save) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = player_offsets_map.find(std::string(player_data_to_save.username));
    if (it == player_offsets_map.end())
        return;

    std::fstream data_file(PLAYERS_DATA_FILENAME,
                           std::ios::in | std::ios::out | std::ios::binary);
    if (!data_file.is_open())
        return;

    data_file.seekp(static_cast<std::streamoff>(it->second));
    if (!data_file.good())
        return;

    data_file.write(reinterpret_cast<const char*>(&player_data_to_save),
                    sizeof(PlayerData));
}
