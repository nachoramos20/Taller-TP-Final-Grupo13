#include "ChatCheatCommands.h"

#include <cstring>

#include "../Items.h"
#include "../config/GameConfig.h"
#include "../entities/PlayerData.h"

ChatCheatCommands::ChatCheatCommands(uint16_t client_id): client_id_(client_id) {}

void ChatCheatCommands::set_nivel(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int nivel = std::stoi(args);
        if (nivel < 1 || nivel > 100) {
            world.push_message(client_id_, 0, "El nivel debe estar entre 1 y 100.");
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
        world.push_message(client_id_, 0, "Nivel seteado a " + std::to_string(nivel) + ".");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /set-nivel <1-100>");
    }
}

void ChatCheatCommands::set_vida(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int valor = std::stoi(args);
        if (valor < 0) {
            world.push_message(client_id_, 0, "La vida no puede ser negativa.");
            return;
        }
        p->max_hp = static_cast<uint16_t>(valor);
        p->hp = p->max_hp;
        world.push_message(client_id_, 0, "Vida máxima seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /set-vida <cantidad>");
    }
}

void ChatCheatCommands::set_fuerza(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id_, 0, "La fuerza debe estar entre 0 y 100.");
            return;
        }
        p->strength = static_cast<uint16_t>(valor);
        world.push_message(client_id_, 0, "Fuerza seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /set-fuerza <0-100>");
    }
}

void ChatCheatCommands::set_agilidad(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id_, 0, "La agilidad debe estar entre 0 y 100.");
            return;
        }
        p->agility = static_cast<uint16_t>(valor);
        world.push_message(client_id_, 0, "Agilidad seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /set-agilidad <0-100>");
    }
}

void ChatCheatCommands::set_inteligencia(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id_, 0, "La inteligencia debe estar entre 0 y 100.");
            return;
        }
        p->intelligence = static_cast<uint16_t>(valor);
        world.push_message(client_id_, 0, "Inteligencia seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /set-inteligencia <0-100>");
    }
}

void ChatCheatCommands::set_constitucion(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int valor = std::stoi(args);
        if (valor < 0 || valor > 100) {
            world.push_message(client_id_, 0, "La constitución debe estar entre 0 y 100.");
            return;
        }
        p->constitution = static_cast<uint16_t>(valor);
        world.push_message(client_id_, 0, "Constitución seteada a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /set-constitucion <0-100>");
    }
}

void ChatCheatCommands::morir_instantaneo(World& world) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    if (p->is_ghost) {
        world.push_message(client_id_, 0, "Ya estás muerto.");
        return;
    }
    p->hp = 0;
    p->is_ghost = true;
    p->meditating = false;
    world.drop_player_loot(*p);
    world.push_message(client_id_, 0,
                       "Has muerto instantáneamente. Tus pertenencias han caído al suelo.");
}

void ChatCheatCommands::revivir_instantaneo(World& world) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    if (!p->is_ghost) {
        world.push_message(client_id_, 0, "No estás muerto.");
        return;
    }
    p->is_ghost = false;
    p->hp = p->max_hp;
    p->mp = p->max_mp;
    // Si tiene clan, notificar que volvió
    if (std::strlen(p->clan_name) > 0) {
        world.clan_notify_login(client_id_, true);
    }
    world.push_message(client_id_, 0, "Has revivido instantáneamente.");
}

void ChatCheatCommands::obtener_objeto(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int id = std::stoi(args);
        if (id < 1 || id > 255) {
            world.push_message(client_id_, 0, "ID de objeto inválido.");
            return;
        }
        ItemId iid = static_cast<ItemId>(id);
        if (!Items::exists(iid)) {
            world.push_message(client_id_, 0,
                               "No existe un objeto con ID " + std::to_string(id) + ".");
            return;
        }
        int free_slot = -1;
        for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
            if (p->inventory[i] == 0) {
                free_slot = i;
                break;
            }
        }
        if (free_slot == -1) {
            world.push_message(client_id_, 0, "Inventario lleno.");
            return;
        }
        p->inventory[free_slot] = static_cast<uint8_t>(id);
        const ItemDef& def = Items::get(iid);
        world.push_message(client_id_, 0,
                           "Has obtenido " + def.name + " (ID " + std::to_string(id) + ").");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /obtener-objeto <id>");
    }
}

void ChatCheatCommands::set_oro(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id_);
    if (!p)
        return;
    try {
        int valor = std::stoi(args);
        if (valor < 0) {
            world.push_message(client_id_, 0, "El oro no puede ser negativo.");
            return;
        }
        p->gold = static_cast<uint32_t>(valor);
        world.push_message(client_id_, 0, "Oro seteado a " + std::to_string(valor) + ".");
    } catch (...) {
        world.push_message(client_id_, 0, "Uso: /set-oro <cantidad>");
    }
}
