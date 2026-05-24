#pragma once

#include <cstdint>
#include <string>
#include "../socket.h"
#include "protocol.h"
#include "dtos.h"

class Deserializer {
public:
    explicit Deserializer(Socket& socket) : _socket(socket) {}

    MsgType recv_opcode() {
        return static_cast<MsgType>(recv_uint8());
    }

    uint16_t recv_login_ok() {
        return recv_uint16();
    }

    std::string recv_login_error() {
        return recv_str8();
    }

    SnapshotDTO recv_snapshot() {
        SnapshotDTO s;

        s.tick           = recv_uint32();
        s.self_entity_id = recv_uint16();
        s.hp             = recv_uint16();
        s.max_hp         = recv_uint16();
        s.mp             = recv_uint16();
        s.max_mp         = recv_uint16();
        s.exp            = recv_uint32();
        s.level          = recv_uint8();
        s.gold           = recv_uint32();
        s.is_ghost       = recv_uint8();
        s.meditating     = recv_uint8();

        uint8_t inv_count = recv_uint8();
        for (int i = 0; i < inv_count && i < SnapshotDTO::INVENTORY_SIZE; i++)
            s.inventory[i] = recv_uint8();
        s.equipped_wpn  = recv_uint8();
        s.equipped_arm  = recv_uint8();
        s.equipped_helm = recv_uint8();
        s.equipped_shld = recv_uint8();

        uint8_t entity_count = recv_uint8();
        s.entities.resize(entity_count);
        for (auto& e : s.entities) {
            e.entity_id   = recv_uint16();
            e.entity_type = recv_uint8();
            e.pos_x       = recv_uint16();
            e.pos_y       = recv_uint16();
            e.direction   = recv_uint8();
            e.sprite_id   = recv_uint8();
            e.is_ghost    = recv_uint8();
            e.hp_pct      = recv_uint8();
        }

        uint8_t msg_count = recv_uint8();
        s.messages.resize(msg_count);
        for (auto& m : s.messages) {
            m.msg_type = recv_uint8();
            m.text     = recv_str8();
        }

        return s;
    }

private:
    uint8_t recv_uint8() {
        uint8_t val;
        _socket.recvall(&val, 1);
        return val;
    }

    uint16_t recv_uint16() {
        uint8_t buf[2];
        _socket.recvall(buf, 2);
        return (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
    }

    uint32_t recv_uint32() {
        uint8_t buf[4];
        _socket.recvall(buf, 4);
        return (static_cast<uint32_t>(buf[0]) << 24) |
               (static_cast<uint32_t>(buf[1]) << 16) |
               (static_cast<uint32_t>(buf[2]) <<  8) |
                static_cast<uint32_t>(buf[3]);
    }

    std::string recv_str8() {
        uint8_t len = recv_uint8();
        std::string s(len, '\0');
        _socket.recvall(s.data(), len);
        return s;
    }

    Socket& _socket;
};