#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "../../common/protocol/MapaDTO.h"
#include "../../common/protocol/Protocol.h"
#include "../../common/protocol/dtos.h"
#include "../../common/protocol/protocol.h"

// Agrupa todo lo que el cliente manda al server
// (login/registro, movimiento, comandos de inventario y chat, hechizos,
// cheats) y lo que recibe de él (resultado de login, mapa, snapshots).
// SenderThread y ReceiverThread usan una misma instancia compartida.
class ClientProtocol: public Protocol {
public:
    explicit ClientProtocol(Socket&& socket);

    void send_login(const std::string& username);
    void send_register(const std::string& username, Race race, Class cls);
    void send_move(uint16_t pos_x, uint16_t pos_y);
    void send_attack(uint16_t target_id);
    void send_chat_command(const std::string& command_text);
    void send_equip_item(uint8_t inv_slot);
    void send_unequip_item(EquipSlot slot);
    void send_move_item(uint8_t from_slot, uint8_t to_slot);
    void send_drop_item(uint8_t inv_slot);
    void send_pick_item();
    void send_use_item(uint8_t inv_slot);
    void send_meditate();
    void send_resurrect();
    void send_npc_interact(uint16_t npc_id);
    void send_logout();
    void send_cast_spell(uint16_t target_id, uint8_t spell_id);
    void send_cheat(uint8_t cheat_id);

    MsgType recv_opcode();
    uint16_t recv_login_ok();
    std::string recv_login_error();
    MapaDTO recv_map();
    SnapshotDTO recv_snapshot();
};
