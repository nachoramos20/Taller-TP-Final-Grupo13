#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "../../common/protocol/Protocol.h"
#include "../../common/protocol/MapaDTO.h"
#include "../../common/protocol/protocol.h"
#include "../game/commands/Commands.h"

// Pasa apenas el umbral de 200 líneas de implementación con 3 grupos de
// métodos (handshake, parsing de comandos, envío de mapa/snapshot), pero
// no se separan en clases auxiliares: los tres necesitan los primitivos
// send_uint8/recv_uint16/etc. de Protocol, que son `protected` (solo
// accesibles por subclases). Una clase auxiliar tendría que heredar de
// Protocol igual que ServerProtocol, duplicando la relación de herencia
// sin ganar nada — o forzar esos primitivos a `public`, debilitando el
// encapsulamiento del framing del protocolo para todo el proyecto.
class ServerProtocol : public Protocol {
public:
    explicit ServerProtocol(Socket&& socket);

    std::shared_ptr<ServerCommand> receive_command(uint16_t client_id);
    MsgType receive_handshake();
    void handshake_login(std::string& username);
    void handshake_register(std::string& username, uint8_t& race_id, uint8_t& class_id);

    void send_login_ok(uint16_t entity_id);
    void send_login_error(const std::string& error_message);
    void send_mapa(const MapaDTO& map_data);
    void send_snapshot(const SnapshotDTO& snapshot);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
    ServerProtocol(ServerProtocol&&) = default;
    ServerProtocol& operator=(ServerProtocol&&) = default;
    ~ServerProtocol() override = default;

private:
    std::shared_ptr<MoveCommand>        receive_move_command(uint16_t client_id);
    std::shared_ptr<AttackCommand>      receive_attack(uint16_t client_id);
    std::shared_ptr<EquipCommand>       receive_equip(uint16_t client_id);
    std::shared_ptr<UnequipCommand>     receive_unequip(uint16_t client_id);
    std::shared_ptr<DropCommand>        receive_drop(uint16_t client_id);
    std::shared_ptr<MoveItemCommand>    receive_move_item(uint16_t client_id);
    std::shared_ptr<PickCommand>        receive_pick(uint16_t client_id);
    std::shared_ptr<UseItemCommand>     receive_use(uint16_t client_id);
    std::shared_ptr<MeditateCommand>    receive_meditate(uint16_t client_id);
    std::shared_ptr<ResurrectCommand>   receive_resurrect(uint16_t client_id);
    std::shared_ptr<LogoutCommand>      receive_logout(uint16_t client_id);
    std::shared_ptr<ChatCommand>        receive_chat_command(uint16_t client_id);
    std::shared_ptr<NpcInteractCommand> receive_npc_interact(uint16_t client_id);
    std::shared_ptr<CastSpellCommand>   receive_cast_spell(uint16_t client_id);
    std::shared_ptr<CheatCommand>       receive_cheat(uint16_t client_id);

    // Factory + tabla de dispatch por MsgType (en vez del switch de 14
    // casos que llamaba a cada receive_*): cada entrada envuelve un
    // receive_* en una firma uniforme que devuelve el ServerCommand base.
    using CommandFactory = std::function<std::shared_ptr<ServerCommand>(ServerProtocol&, uint16_t)>;
    static const std::unordered_map<MsgType, CommandFactory>& dispatch_table();
};
