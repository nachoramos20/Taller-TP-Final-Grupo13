#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "../socket.h"
#include "protocol.h"
#include "dtos.h"
#include "../MapaDTO.h"

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

    void send_move_item(uint8_t from_slot, uint8_t to_slot) {
        send_uint8(static_cast<uint8_t>(MsgType::MOVE_ITEM));
        send_uint8(from_slot);
        send_uint8(to_slot);
    }

    void send_cheat(uint8_t cheat_id) {
        send_uint8(static_cast<uint8_t>(MsgType::CHEAT));
        send_uint8(cheat_id);
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

    void send_use_item(uint8_t inv_slot) {
        send_uint8(static_cast<uint8_t>(MsgType::USE_ITEM));
        send_uint8(inv_slot);
    }

    void send_meditate() {
        send_uint8(static_cast<uint8_t>(MsgType::MEDITATE));
    }

    void send_resurrect() {
        send_uint8(static_cast<uint8_t>(MsgType::RESURRECT));
    }

    void send_npc_interact(uint16_t npc_id) {
        send_uint8(static_cast<uint8_t>(MsgType::NPC_INTERACT));
        send_uint16(npc_id);
    }

    void send_logout() {
        send_uint8(static_cast<uint8_t>(MsgType::LOGOUT));
    }

    void send_login_ok(uint16_t entity_id) {
        send_uint8(static_cast<uint8_t>(MsgType::LOGIN_OK));
        send_uint16(entity_id);
    }

    void send_login_error(const std::string& msg) {
        send_uint8(static_cast<uint8_t>(MsgType::LOGIN_ERROR));
        send_str8(msg);
    }

    void send_map(const MapaDTO& map) {
        send_uint8(static_cast<uint8_t>(MsgType::MAPA));
        send_uint16(map.width);
        send_uint16(map.height);
        send_uint32(static_cast<uint32_t>(map.tiles.size()));

        std::vector<uint8_t> buf;
        buf.reserve(map.tiles.size() * 6);
        for (const auto& tile : map.tiles) {
            buf.push_back(tile.floor_id >> 8);
            buf.push_back(tile.floor_id & 0xFF);
            buf.push_back(tile.object_id >> 8);
            buf.push_back(tile.object_id & 0xFF);
            buf.push_back(tile.object_superior_id >> 8);
            buf.push_back(tile.object_superior_id & 0xFF);
        }
        _socket.sendall(buf.data(), buf.size());
    }

    void send_snapshot(const SnapshotDTO& snap) {
        send_uint8(static_cast<uint8_t>(MsgType::SNAPSHOT));
        send_uint32(snap.tick);
        send_uint16(snap.self_entity_id);
        send_uint16(snap.hp);
        send_uint16(snap.max_hp);
        send_uint16(snap.mp);
        send_uint16(snap.max_mp);
        send_uint32(snap.exp);
        send_uint8(snap.level);
        send_uint8(snap.cls);
        send_uint32(snap.gold);
        send_uint8(snap.is_ghost);
        send_uint8(snap.meditating);

        send_uint8(static_cast<uint8_t>(SnapshotDTO::INVENTORY_SIZE));
        for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
            send_uint8(snap.inventory[i]);
        send_uint8(snap.equipped_wpn);
        send_uint8(snap.equipped_arm);
        send_uint8(snap.equipped_helm);
        send_uint8(snap.equipped_shld);

        send_uint8(static_cast<uint8_t>(snap.entities->size()));
        for (const auto& e : *snap.entities) {
            send_uint16(e.entity_id);
            send_uint8(e.entity_type);
            send_uint16(e.pos_x);
            send_uint16(e.pos_y);
            send_uint8(e.direction);
            send_uint8(e.sprite_id);
            send_uint8(e.is_ghost);
            send_uint8(e.hp_pct);
            send_str8(e.username);
        }

        send_uint8(static_cast<uint8_t>(snap.messages->size()));
        for (const auto& m : *snap.messages) {
            send_uint8(m.msg_type);
            send_str8(m.text);
        }
    }

void send_cast_spell(uint16_t target_id, uint8_t spell_id) {
    send_uint8(static_cast<uint8_t>(MsgType::CAST_SPELL));
    send_uint16(target_id);
    send_uint8(spell_id);
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

    void send_uint32(uint32_t value) {
        uint8_t buf[4] = {
            static_cast<uint8_t>(value >> 24),
            static_cast<uint8_t>(value >> 16),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value & 0xFF)
        };
        _socket.sendall(buf, 4);
    }

    void send_str8(const std::string& s) {
        uint8_t len = static_cast<uint8_t>(s.size());
        send_uint8(len);
        _socket.sendall(s.data(), len);
    }

    Socket& _socket;
};