#pragma once

#include <cstdint>
#include <string>
#include "../socket.h"
#include "protocol.h"

class Serializer {
public:
    explicit Serializer(Socket& socket) : _socket(socket) {}

    void send_login(const std::string& username) {
        send_uint8(static_cast<uint8_t>(MsgType::LOGIN));
        send_str8(username);
    }

    void send_register(const std::string& username, Race race, Class cls) {
        send_uint8(static_cast<uint8_t>(MsgType::REGISTER));
        send_str8(username);
        send_uint8(static_cast<uint8_t>(race));
        send_uint8(static_cast<uint8_t>(cls));
    }

    void send_move(uint16_t pos_x, uint16_t pos_y) {
        send_uint8(static_cast<uint8_t>(MsgType::MOVE));
        send_uint16(pos_x);
        send_uint16(pos_y);
    }

    void send_attack(uint16_t target_id) {
        send_uint8(static_cast<uint8_t>(MsgType::ATTACK));
        send_uint16(target_id);
    }

    void send_chat_command(const std::string& cmd) {
        send_uint8(static_cast<uint8_t>(MsgType::CHAT_COMMAND));
        send_str8(cmd);
    }

    void send_equip_item(uint8_t inv_slot) {
        send_uint8(static_cast<uint8_t>(MsgType::EQUIP_ITEM));
        send_uint8(inv_slot);
    }

    void send_unequip_item(EquipSlot slot) {
        send_uint8(static_cast<uint8_t>(MsgType::UNEQUIP_ITEM));
        send_uint8(static_cast<uint8_t>(slot));
    }

    void send_drop_item(uint8_t inv_slot) {
        send_uint8(static_cast<uint8_t>(MsgType::DROP_ITEM));
        send_uint8(inv_slot);
    }

    void send_pick_item() {
        send_uint8(static_cast<uint8_t>(MsgType::PICK_ITEM));
    }

    void send_npc_interact(uint16_t npc_id) {
        send_uint8(static_cast<uint8_t>(MsgType::NPC_INTERACT));
        send_uint16(npc_id);
    }

    void send_logout() {
        send_uint8(static_cast<uint8_t>(MsgType::LOGOUT));
    }

private:
    void send_uint8(uint8_t value) {
        _socket.sendall(&value, 1);
    }

    void send_uint16(uint16_t value) {
        uint8_t buf[2] = {
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value & 0xFF)
        };
        _socket.sendall(buf, 2);
    }

    void send_str8(const std::string& s) {
        uint8_t len = static_cast<uint8_t>(s.size());
        send_uint8(len);
        _socket.sendall(s.data(), len);
    }

    Socket& _socket;
};