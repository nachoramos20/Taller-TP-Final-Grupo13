#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "../../common/socket.h"
#include "../../common/MapaDTO.h"
#include "../../common/protocol/protocol.h"
#include "../game/Commands.h"

class ServerProtocol {
public:
    explicit ServerProtocol(Socket&& socket);

    std::shared_ptr<ServerCommand> receive_command(uint16_t client_id);
    MsgType receive_handshake();
    void handshake_login(std::string& username);
    void handshake_register(std::string& username, uint8_t& race, uint8_t& cls);

    void send_login_ok(uint16_t entity_id);
    void send_login_error(const std::string& msg);
    void send_mapa(const MapaDTO& mapa);
    void send_snapshot(const SnapshotDTO& snap);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
    ServerProtocol(ServerProtocol&&) = default;
    ServerProtocol& operator=(ServerProtocol&&) = default;
    virtual ~ServerProtocol() = default;

    void shutdown(int how);

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
    std::shared_ptr<CastSpellCommand> receive_cast_spell(uint16_t client_id);

    uint8_t  recv_uint8();
    uint16_t recv_uint16();
    std::string recv_str8();

    void send_uint8(uint8_t v);
    void send_uint16(uint16_t v);
    void send_uint32(uint32_t v);
    void send_str8(const std::string& s);

    Socket socket;
};
