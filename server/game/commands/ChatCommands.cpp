#include "Commands.h"
#include "../Items.h"
#include "../config/GameConfig.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <vector>

// Patrón aplicado: Command + tabla de dispatch (en vez de la cadena de 31
// if/else if que despachaba por texto de comando). Cada entrada de la
// tabla envuelve un handle_* existente (sin tocar su lógica) detrás de la
// misma firma uniforme, así agregar un comando nuevo es agregar una fila
// a la tabla, no una rama más a una cadena que ya no entraba en una
// pantalla. La tabla vive en una función-miembro privada (dispatch_table())
// para poder llamar a los handle_* privados; es `static const` local, así
// que se construye una sola vez en todo el proceso aunque ChatCommand se
// instancie una vez por cada mensaje de chat recibido.
//
// Puerta del cementerio: ocupa dos tiles de ancho en X (60 y 61) a la
// altura y=62. Es el único punto de entrada a la mazmorra (ver
// handle_entrar_mazmorra) y lo que describe handle_info_mazmorra.
namespace {
    constexpr uint16_t CEMENTERIO_PUERTA_X1 = 60;
    constexpr uint16_t CEMENTERIO_PUERTA_X2 = 61;
    constexpr uint16_t CEMENTERIO_PUERTA_Y  = 62;
    constexpr uint16_t CEMENTERIO_PUERTA_RADIO = 2;
}

ChatCommand::ChatCommand(uint16_t c, std::string cmd)
    : client_id(c), cmd(std::move(cmd)) {}

const ChatCommand::DispatchTable& ChatCommand::dispatch_table() {
    static const DispatchTable table = {
        {"/meditar",            [](ChatCommand& s, World& w, const std::string&)    { s.handle_meditar(w); }},
        {"/resucitar",          [](ChatCommand& s, World& w, const std::string&)    { s.handle_resucitar(w); }},
        {"/curar",              [](ChatCommand& s, World& w, const std::string&)    { s.handle_curar(w); }},
        {"/depositar",          [](ChatCommand& s, World& w, const std::string& a)  { s.handle_depositar(w, a); }},
        {"/retirar",            [](ChatCommand& s, World& w, const std::string& a)  { s.handle_retirar(w, a); }},
        {"/listar",             [](ChatCommand& s, World& w, const std::string&)    { s.handle_listar(w); }},
        {"/comprar",            [](ChatCommand& s, World& w, const std::string& a)  { s.handle_comprar(w, a); }},
        {"/vender",             [](ChatCommand& s, World& w, const std::string& a)  { s.handle_vender(w, a); }},
        {"/tomar",              [](ChatCommand& s, World& w, const std::string&)    { s.handle_tomar(w); }},
        {"/tirar",              [](ChatCommand& s, World& w, const std::string& a)  { s.handle_tirar(w, a); }},
        {"/fundar-clan",        [](ChatCommand& s, World& w, const std::string& a)  { s.handle_fundar_clan(w, a); }},
        {"/unirse",             [](ChatCommand& s, World& w, const std::string& a)  { s.handle_unirse(w, a); }},
        {"/revisar-clan",       [](ChatCommand& s, World& w, const std::string&)    { s.handle_revisar_clan(w); }},
        {"/clan-aceptar",       [](ChatCommand& s, World& w, const std::string& a)  { s.handle_clan_aceptar(w, a); }},
        {"/clan-rechazar",      [](ChatCommand& s, World& w, const std::string& a)  { s.handle_clan_rechazar(w, a); }},
        {"/clan-ban",           [](ChatCommand& s, World& w, const std::string& a)  { s.handle_clan_ban(w, a); }},
        {"/clan-kick",          [](ChatCommand& s, World& w, const std::string& a)  { s.handle_clan_kick(w, a); }},
        {"/dejar-clan",         [](ChatCommand& s, World& w, const std::string&)    { s.handle_dejar_clan(w); }},
        {"/entrar-mazmorra",    [](ChatCommand& s, World& w, const std::string&)    { s.handle_entrar_mazmorra(w); }},
        {"/salir-mazmorra",     [](ChatCommand& s, World& w, const std::string&)    { s.handle_salir_mazmorra(w); }},
        {"/info-mazmorra",      [](ChatCommand& s, World& w, const std::string&)    { s.handle_info_mazmorra(w); }},
        {"/set-nivel",          [](ChatCommand& s, World& w, const std::string& a)  { s.handle_set_nivel(w, a); }},
        {"/set-vida",           [](ChatCommand& s, World& w, const std::string& a)  { s.handle_set_vida(w, a); }},
        {"/set-fuerza",         [](ChatCommand& s, World& w, const std::string& a)  { s.handle_set_fuerza(w, a); }},
        {"/set-agilidad",       [](ChatCommand& s, World& w, const std::string& a)  { s.handle_set_agilidad(w, a); }},
        {"/set-inteligencia",   [](ChatCommand& s, World& w, const std::string& a)  { s.handle_set_inteligencia(w, a); }},
        {"/set-constitucion",   [](ChatCommand& s, World& w, const std::string& a)  { s.handle_set_constitucion(w, a); }},
        {"/morir-instantaneo",  [](ChatCommand& s, World& w, const std::string&)    { s.handle_morir_instantaneo(w); }},
        {"/revivir-instantaneo",[](ChatCommand& s, World& w, const std::string&)    { s.handle_revivir_instantaneo(w); }},
        {"/obtener-objeto",     [](ChatCommand& s, World& w, const std::string& a)  { s.handle_obtener_objeto(w, a); }},
        {"/set-oro",            [](ChatCommand& s, World& w, const std::string& a)  { s.handle_set_oro(w, a); }},
    };
    return table;
}

void ChatCommand::execute(World& world) {
    std::istringstream ss(cmd);
    std::string token;
    ss >> token;

    if (!token.empty() && token[0] == '@') {
        handle_private_msg(world, cmd);
        return;
    }

    std::string rest;
    std::getline(ss, rest);
    if (!rest.empty() && rest[0] == ' ') rest = rest.substr(1);

    const auto& table = dispatch_table();
    auto it = table.find(token);
    if (it == table.end()) {
        world.push_message(client_id, 0, "Comando desconocido: " + token);
        return;
    }
    it->second(*this, world, rest);
}

void ChatCommand::handle_meditar(World& world) {
    MeditateCommand(client_id).execute(world);
}

void ChatCommand::handle_resucitar(World& world) {
    // El chequeo de cercanía al sacerdote (y cuál de los dos) vive en
    // ResurrectCommand::execute, compartido con la tecla R.
    ResurrectCommand(client_id).execute(world);
}

void ChatCommand::handle_curar(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;

    if (!world.player_near_service_npc(client_id, NpcId::PRIEST)) {
        world.push_message(client_id, 0,
            "Debes estar cerca del Sacerdote para ser curado.\n"
            "Acércate y haz click en él primero.");
        return;
    }
    if (p->is_ghost) {
        world.push_message(client_id, 0, "Eres un fantasma. Usa /resucitar primero.");
        return;
    }
    p->hp = p->max_hp;
    p->mp = p->max_mp;
    world.push_message(client_id, 0, "El Sacerdote te curó completamente.");
}

void ChatCommand::handle_comprar(World& world, const std::string& item_name) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    if (!world.player_near_service_npc(client_id, NpcId::MERCHANT)) {
        world.push_message(client_id, 0,
            "Debes estar cerca del Comerciante para comprar.");
        return;
    }

    // Catálogo según la zona del comerciante más cercano (tabla compartida
    // con handle_listar_comerciante, ver merchant_catalog_for_zone).
    uint8_t zone = world.get_nearby_merchant_zone(client_id);
    std::vector<ItemId> shop_items = merchant_catalog_for_zone(zone);

    int free_slot = -1;
    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i)
        if (p->inventory[i] == 0) { free_slot = i; break; }
    if (free_slot == -1) {
        world.push_message(client_id, 0, "Inventario lleno.");
        return;
    }

    for (ItemId iid : shop_items) {
        const ItemDef& def = Items::get(iid);
        if (Items::name_equals_ci(def.name, item_name)) {
            uint32_t price = (static_cast<uint32_t>(def.min_value) + def.max_value) * 8 + 50;
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
    world.push_message(client_id, 0, "El comerciante no tiene ese articulo. Usa /listar.");
}

void ChatCommand::handle_vender(World& world, const std::string& item_name) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    if (!world.player_near_service_npc(client_id, NpcId::MERCHANT)) {
        world.push_message(client_id, 0,
            "Debes estar cerca del Comerciante para vender.\n"
            "Acércate y haz click en él primero.");
        return;
    }

    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) continue;
        if (!Items::exists(static_cast<ItemId>(p->inventory[i]))) continue;
        const ItemDef& def = Items::get(static_cast<ItemId>(p->inventory[i]));
        if (Items::name_equals_ci(def.name, item_name)) {
            uint32_t sell_price = (def.max_value * 10 + 10) / 2;
            p->gold += sell_price;
            p->inventory[i] = 0;

            if (p->equipped_weapon == static_cast<uint8_t>(i)) p->equipped_weapon = 0xFF;
            if (p->equipped_armor  == static_cast<uint8_t>(i)) p->equipped_armor  = 0xFF;
            if (p->equipped_helmet == static_cast<uint8_t>(i)) p->equipped_helmet = 0xFF;
            if (p->equipped_shield == static_cast<uint8_t>(i)) p->equipped_shield = 0xFF;

            world.push_message(client_id, 0, "Vendiste " + item_name + " por " + std::to_string(sell_price) + " de oro.");
            return;
        }
    }
    world.push_message(client_id, 0, "No tienes ese objeto para vender.");
}

void ChatCommand::handle_tomar(World& world) {
    PickCommand(client_id).execute(world);
}

void ChatCommand::handle_tirar(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;

    try {
        int slot = std::stoi(args);
        if (slot >= 0 && slot < PlayerData::INVENTORY_SIZE) {
            DropCommand(client_id, static_cast<uint8_t>(slot)).execute(world);
            return;
        }
    } catch (...) {}

    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0) continue;
        if (Items::exists(static_cast<ItemId>(p->inventory[i]))) {
            if (Items::name_equals_ci(Items::get(static_cast<ItemId>(p->inventory[i])).name, args)) {
                DropCommand(client_id, static_cast<uint8_t>(i)).execute(world);
                return;
            }
        }
    }
    world.push_message(client_id, 0, "No encontraste ese objeto en tu inventario.");
}

void ChatCommand::handle_private_msg(World& world, const std::string& full_cmd) {
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

void ChatCommand::handle_entrar_mazmorra(World& world) {
    PlayerData* player = world.get_player_mutable(client_id);
    if (!player) {
        return;
    }

    if (player->is_ghost) {
        world.push_message(client_id, 0, "No puedes entrar a la mazmorra estando muerto.");
        return;
    }

    int player_x = static_cast<int>(player->pos_x);
    int player_y = static_cast<int>(player->pos_y);
    int dx = std::min(std::abs(player_x - static_cast<int>(CEMENTERIO_PUERTA_X1)),
                      std::abs(player_x - static_cast<int>(CEMENTERIO_PUERTA_X2)));
    int dy = std::abs(player_y - static_cast<int>(CEMENTERIO_PUERTA_Y));
    if (dx > CEMENTERIO_PUERTA_RADIO || dy > CEMENTERIO_PUERTA_RADIO) {
        world.push_message(client_id, 0, "Debes estar en la puerta del cementerio para entrar a la mazmorra.");
        return;
    }

    uint16_t spawn_mazmorra_x = 114;
    uint16_t spawn_mazmorra_y = 79;
    
    Mazmorra* mazmorra = world.get_dungeon_at(spawn_mazmorra_x, spawn_mazmorra_y);
    if (!mazmorra) {
        world.push_message(client_id, 0, "Mazmorra no encontrada");
        return;
    }
    if (mazmorra->in_mazmorra(player->pos_x, player->pos_y)) {
        world.push_message(client_id, 0, "Ya estás dentro de la mazmorra.");
        return;
    }

    mazmorra->player_entered();
    
    world.tp_player(client_id, spawn_mazmorra_x, spawn_mazmorra_y);
    world.push_message(client_id, 0, "Has entrado a la mazmorra.");
}

void ChatCommand::handle_salir_mazmorra(World& world) {
    PlayerData* player = world.get_player_mutable(client_id);
    if (!player) return;

    Mazmorra* mazmorra = world.get_dungeon_at(player->pos_x, player->pos_y);
    if (!mazmorra) {
        world.push_message(client_id, 0, "No estás dentro de una mazmorra.");
        return;
    }

    mazmorra->player_left();

    uint16_t portal_spawn_x = 60;
    uint16_t portal_spawn_y = 62;
    world.tp_player(client_id, portal_spawn_x, portal_spawn_y);

    world.push_message(client_id, 0, "Has salido de la mazmorra.");
}

void ChatCommand::handle_info_mazmorra(World& world) {
    world.push_message(client_id, 0,
        "La entrada a la mazmorra esta en la puerta de la casa del cementerio. Parate ahi y usa /entrar-mazmorra.");
}

// ---- Cheats ----

void ChatCommand::handle_set_nivel(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int nivel = std::stoi(args);
        if (nivel < 1 || nivel > 100) {
            world.push_message(client_id, 0, "El nivel debe estar entre 1 y 100.");
            return;
        }
        p->level = static_cast<uint8_t>(nivel);
        // Recalcular max_hp/max_mp según el nuevo nivel
        uint16_t base_hp = GameConfig::get().initial_max_hp(p->race, p->cls);
        uint16_t base_mp = GameConfig::get().initial_max_mp(p->race, p->cls);
        p->max_hp = base_hp + (nivel - 1) * 10;
        p->max_mp = base_mp + (nivel - 1) * 5;
        p->hp = p->max_hp;
        p->mp = p->max_mp;
        world.push_message(client_id, 0, "Nivel seteado a " + std::to_string(nivel) + ".");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /set-nivel <1-100>");
    }
}

void ChatCommand::handle_set_vida(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int valor = std::stoi(args);
        if (valor < 0) {
            world.push_message(client_id, 0, "La vida no puede ser negativa.");
            return;
        }
        p->max_hp = static_cast<uint16_t>(valor);
        p->hp = p->max_hp;
        world.push_message(client_id, 0, "Vida máxima seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /set-vida <cantidad>");
    }
}

void ChatCommand::handle_set_fuerza(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id, 0, "La fuerza debe estar entre 0 y 100.");
            return;
        }
        p->strength = static_cast<uint16_t>(valor);
        world.push_message(client_id, 0, "Fuerza seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /set-fuerza <0-100>");
    }
}

void ChatCommand::handle_set_agilidad(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id, 0, "La agilidad debe estar entre 0 y 100.");
            return;
        }
        p->agility = static_cast<uint16_t>(valor);
        world.push_message(client_id, 0, "Agilidad seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /set-agilidad <0-100>");
    }
}

void ChatCommand::handle_set_inteligencia(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id, 0, "La inteligencia debe estar entre 0 y 100.");
            return;
        }
        p->intelligence = static_cast<uint16_t>(valor);
        world.push_message(client_id, 0, "Inteligencia seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /set-inteligencia <0-100>");
    }
}

void ChatCommand::handle_set_constitucion(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id, 0, "La constitución debe estar entre 0 y 100.");
            return;
        }
        p->constitution = static_cast<uint16_t>(valor);
        world.push_message(client_id, 0, "Constitución seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /set-constitucion <0-100>");
    }
}

void ChatCommand::handle_morir_instantaneo(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    if (p->is_ghost) {
        world.push_message(client_id, 0, "Ya estás muerto.");
        return;
    }
    p->hp = 0;
    p->is_ghost = true;
    p->meditating = false;
    world.drop_player_loot(*p);
    world.push_message(client_id, 0, "Has muerto instantáneamente. Tus pertenencias han caído al suelo.");
}

void ChatCommand::handle_revivir_instantaneo(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    if (!p->is_ghost) {
        world.push_message(client_id, 0, "No estás muerto.");
        return;
    }
    p->is_ghost = false;
    p->hp = p->max_hp;
    p->mp = p->max_mp;
    // Si tiene clan, notificar que volvió
    if (std::strlen(p->clan_name) > 0) {
        world.clan_notify_login(client_id, true);
    }
    world.push_message(client_id, 0, "Has revivido instantáneamente.");
}

void ChatCommand::handle_obtener_objeto(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int id = std::stoi(args);
        if (id < 1 || id > 255) {
            world.push_message(client_id, 0, "ID de objeto inválido.");
            return;
        }
        ItemId iid = static_cast<ItemId>(id);
        if (!Items::exists(iid)) {
            world.push_message(client_id, 0, "No existe un objeto con ID " + std::to_string(id) + ".");
            return;
        }
        int free_slot = -1;
        for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
            if (p->inventory[i] == 0) { free_slot = i; break; }
        }
        if (free_slot == -1) {
            world.push_message(client_id, 0, "Inventario lleno.");
            return;
        }
        p->inventory[free_slot] = static_cast<uint8_t>(id);
        const ItemDef& def = Items::get(iid);
        world.push_message(client_id, 0, "Has obtenido " + def.name + " (ID " + std::to_string(id) + ").");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /obtener-objeto <id>");
    }
}

void ChatCommand::handle_set_oro(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;
    try {
        int valor = std::stoi(args);
        if (valor < 0) {
            world.push_message(client_id, 0, "El oro no puede ser negativo.");
            return;
        }
        p->gold = static_cast<uint32_t>(valor);
        world.push_message(client_id, 0, "Oro seteado a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id, 0, "Uso: /set-oro <cantidad>");
    }
}
