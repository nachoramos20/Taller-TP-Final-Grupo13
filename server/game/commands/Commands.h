#ifndef COMMANDS_H
#define COMMANDS_H

#include "../world/World.h"
#include "../entities/PlayerData.h"
#include "../../../common/protocol/protocol.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// Catálogo de items que vende el comerciante de una zona (0=Ciudad,
// 1=Pueblo, cualquier otro valor = catálogo básico). Compartida por
// ChatCommand::handle_comprar (ChatCommands.cpp) y
// ChatCommand::handle_listar_comerciante (ChatCommand_Bank.cpp) — antes
// cada una tenía su propio switch con el mismo catálogo hardcodeado.
std::vector<ItemId> merchant_catalog_for_zone(uint8_t zone_id);

class ServerCommand {
public:
    virtual void execute(World& world) = 0;
    virtual ~ServerCommand() = default;
};

class MoveCommand : public ServerCommand {
public:
    MoveCommand(uint16_t client_id, uint16_t new_x, uint16_t new_y);
    void execute(World& world) override;
private:
    uint16_t client_id, new_x, new_y;
};

class LoginCommand : public ServerCommand {
public:
    explicit LoginCommand(PlayerData player_data);
    void execute(World& world) override;
    const char* get_username() const;
private:
    PlayerData player_data;
};

class LogoutCommand : public ServerCommand {
public:
    explicit LogoutCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class AttackCommand : public ServerCommand {
public:
    AttackCommand(uint16_t client_id, uint16_t target_id);
    void execute(World& world) override;
private:
    uint16_t client_id, target_id;
};

class AttackNpcCommand : public ServerCommand {
public:
    AttackNpcCommand(uint16_t client_id, uint16_t npc_id);
    void execute(World& world) override;
private:
    uint16_t client_id, npc_id;
};

class EquipCommand : public ServerCommand {
public:
    EquipCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t  inv_slot;
};

class UnequipCommand : public ServerCommand {
public:
    UnequipCommand(uint16_t client_id, EquipSlot slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    EquipSlot slot;
};

class DropCommand : public ServerCommand {
public:
    DropCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t  inv_slot;
};

// Mueve un item de un slot del inventario a otro: si el destino está vacío
// lo mueve, si tiene otro item los intercambia. Actualiza los punteros de
// equipo (equipped_weapon/armor/helmet/shield) si alguno de los dos slots
// estaba equipado, para que sigan apuntando al item correcto.
class MoveItemCommand : public ServerCommand {
public:
    MoveItemCommand(uint16_t client_id, uint8_t from_slot, uint8_t to_slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t  from_slot;
    uint8_t  to_slot;
};

class PickCommand : public ServerCommand {
public:
    explicit PickCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class UseItemCommand : public ServerCommand {
public:
    UseItemCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t  inv_slot;
};

class MeditateCommand : public ServerCommand {
public:
    explicit MeditateCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class ResurrectCommand : public ServerCommand {
public:
    explicit ResurrectCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class NpcInteractCommand : public ServerCommand {
public:
    NpcInteractCommand(uint16_t client_id, uint16_t npc_id);
    void execute(World& world) override;
private:
    uint16_t client_id, npc_id;
};

class ChatCommand : public ServerCommand {
public:
    ChatCommand(uint16_t client_id, std::string cmd);
    void execute(World& world) override;
private:
    uint16_t    client_id;
    std::string cmd;

    // Tabla de dispatch por texto de comando (Command pattern): cada entrada
    // envuelve un handle_* en una firma uniforme. Construida una sola vez
    // (static local en dispatch_table()), evita la cadena de if/else if.
    using DispatchTable = std::unordered_map<std::string,
        std::function<void(ChatCommand&, World&, const std::string&)>>;
    static const DispatchTable& dispatch_table();

    void handle_meditar(World& world);
    void handle_resucitar(World& world);
    void handle_curar(World& world);           
    void handle_depositar(World& world, const std::string& args);
    void handle_retirar(World& world, const std::string& args);
    void handle_listar(World& world);
    void handle_comprar(World& world, const std::string& item_name);
    void handle_vender(World& world, const std::string& item_name);
    void handle_tomar(World& world);
    void handle_tirar(World& world, const std::string& args);
    void handle_private_msg(World& world, const std::string& args); 
    void handle_fundar_clan(World& world, const std::string& clan_name);
    void handle_unirse(World& world, const std::string& clan_name);
    void handle_revisar_clan(World& world);
    void handle_clan_aceptar(World& world, const std::string& nick);
    void handle_clan_rechazar(World& world, const std::string& nick);
    void handle_clan_ban(World& world, const std::string& nick);
    void handle_clan_kick(World& world, const std::string& nick);
    void handle_dejar_clan(World& world);
    void handle_listar_comerciante(World& world);
    void handle_entrar_mazmorra(World& world);
    void handle_salir_mazmorra(World& world);
    void handle_info_mazmorra(World& world);

    // -- Cheats --
    void handle_set_nivel(World& world, const std::string& args);
    void handle_set_vida(World& world, const std::string& args);
    void handle_set_fuerza(World& world, const std::string& args);
    void handle_set_agilidad(World& world, const std::string& args);
    void handle_set_inteligencia(World& world, const std::string& args);
    void handle_set_constitucion(World& world, const std::string& args);
    void handle_morir_instantaneo(World& world);
    void handle_revivir_instantaneo(World& world);
    void handle_obtener_objeto(World& world, const std::string& args);
    void handle_set_oro(World& world, const std::string& args);
};

class CastSpellCommand : public ServerCommand {
public:
    CastSpellCommand(uint16_t client_id, uint16_t target_id, uint8_t spell_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint16_t target_id;
    uint8_t  spell_id;
};

// Cheats disparados por combinación de teclas en el cliente (ver
// InputController y protocol.h::CheatId). Pensados para facilitar pruebas
// (ej. probar /resucitar sin esperar a que un NPC mate al jugador).
class CheatCommand : public ServerCommand {
public:
    CheatCommand(uint16_t client_id, uint8_t cheat_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t  cheat_id;
};

#endif // COMMANDS_H
