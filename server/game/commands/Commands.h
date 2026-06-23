#ifndef COMMANDS_H
#define COMMANDS_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../common/protocol/protocol.h"
#include "../entities/PlayerData.h"
#include "../world/World.h"

// Una acción de un cliente pendiente de aplicar al mundo. Varios hilos
// (uno por cliente) encolan comandos, pero ServerGameLoop es el único que
// llama execute(), siempre desde su propio hilo: las subclases no
// necesitan sincronizarse entre sí.
class ServerCommand {
public:
    virtual void execute(World& world) = 0;
    virtual ~ServerCommand() = default;
};

class MoveCommand: public ServerCommand {
public:
    MoveCommand(uint16_t client_id, uint16_t new_x, uint16_t new_y);
    void execute(World& world) override;

private:
    uint16_t client_id, new_x, new_y;
};

class LoginCommand: public ServerCommand {
public:
    explicit LoginCommand(PlayerData player_data);
    void execute(World& world) override;
    const char* get_username() const;

private:
    PlayerData player_data;
};

class LogoutCommand: public ServerCommand {
public:
    explicit LogoutCommand(uint16_t client_id);
    void execute(World& world) override;

private:
    uint16_t client_id;
};

class AttackCommand: public ServerCommand {
public:
    AttackCommand(uint16_t client_id, uint16_t target_id);
    void execute(World& world) override;

private:
    uint16_t client_id, target_id;
};

class AttackNpcCommand: public ServerCommand {
public:
    AttackNpcCommand(uint16_t client_id, uint16_t npc_id);
    void execute(World& world) override;

private:
    uint16_t client_id, npc_id;
};

class EquipCommand: public ServerCommand {
public:
    EquipCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;

private:
    uint16_t client_id;
    uint8_t inv_slot;
};

class UnequipCommand: public ServerCommand {
public:
    UnequipCommand(uint16_t client_id, EquipSlot slot);
    void execute(World& world) override;

private:
    uint16_t client_id;
    EquipSlot slot;
};

class DropCommand: public ServerCommand {
public:
    DropCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;

private:
    uint16_t client_id;
    uint8_t inv_slot;
};

// Mueve un item de un slot del inventario a otro: si el destino está vacío
// lo mueve, si tiene otro item los intercambia. Actualiza los punteros de
// equipo (equipped_weapon/armor/helmet/shield) si alguno de los dos slots
// estaba equipado, para que sigan apuntando al item correcto.
class MoveItemCommand: public ServerCommand {
public:
    MoveItemCommand(uint16_t client_id, uint8_t from_slot, uint8_t to_slot);
    void execute(World& world) override;

private:
    uint16_t client_id;
    uint8_t from_slot;
    uint8_t to_slot;
};

class PickCommand: public ServerCommand {
public:
    explicit PickCommand(uint16_t client_id);
    void execute(World& world) override;

private:
    uint16_t client_id;
};

class UseItemCommand: public ServerCommand {
public:
    UseItemCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;

private:
    uint16_t client_id;
    uint8_t inv_slot;
};

class MeditateCommand: public ServerCommand {
public:
    explicit MeditateCommand(uint16_t client_id);
    void execute(World& world) override;

private:
    uint16_t client_id;
};

class ResurrectCommand: public ServerCommand {
public:
    explicit ResurrectCommand(uint16_t client_id);
    void execute(World& world) override;

private:
    uint16_t client_id;
};

class NpcInteractCommand: public ServerCommand {
public:
    NpcInteractCommand(uint16_t client_id, uint16_t npc_id);
    void execute(World& world) override;

private:
    uint16_t client_id, npc_id;
};

// Solo conserva los comandos "core" que no encajan en ninguna de las
// clases auxiliares (banco, comerciante, clan, mazmorra, cheats de chat):
// mensajería privada, meditar/resucitar/curar (delegan en otros
// ServerCommand o en el sacerdote) y tomar/tirar items.
class ChatCommand: public ServerCommand {
public:
    ChatCommand(uint16_t client_id, std::string cmd);
    void execute(World& world) override;

private:
    uint16_t client_id;
    std::string cmd;

    // Tabla de dispatch por texto de comando (Command pattern): cada entrada
    // envuelve un handle_* (o una clase auxiliar) en una firma uniforme.
    // Construida una sola vez (static local en dispatch_table()).
    using DispatchTable =
            std::unordered_map<std::string,
                               std::function<void(ChatCommand&, World&, const std::string&)>>;
    static const DispatchTable& dispatch_table();

    void handle_meditar(World& world);
    void handle_resucitar(World& world);
    void handle_curar(World& world);
    // Banco y comerciante son dos respuestas distintas al mismo comando
    // según qué NPC de servicio esté cerca..
    void handle_listar(World& world);
    void handle_tomar(World& world);
    void handle_tirar(World& world, const std::string& args);
    void handle_private_msg(World& world, const std::string& args);
};

class CastSpellCommand: public ServerCommand {
public:
    CastSpellCommand(uint16_t client_id, uint16_t target_id, uint8_t spell_id);
    void execute(World& world) override;

private:
    uint16_t client_id;
    uint16_t target_id;
    uint8_t spell_id;
};

// Cheats disparados por combinación de teclas en el cliente.

class CheatCommand: public ServerCommand {
public:
    CheatCommand(uint16_t client_id, uint8_t cheat_id);
    void execute(World& world) override;

private:
    uint16_t client_id;
    uint8_t cheat_id;
};

#endif  // COMMANDS_H
