#pragma once

#include <cstdint>
#include <string>

#include "../../common/protocol/protocol.h"

// Una acción del jugador pendiente de enviar al servidor (encolada en
// SenderThread). Los factory methods de abajo son la única forma de
// construirlo: cada uno llena solo los campos que ese MsgType necesita.
struct Command {
    MsgType type;

    uint16_t pos_x = 0;
    uint16_t pos_y = 0;
    uint16_t target_id = 0;
    uint8_t slot = 0;
    uint8_t to_slot = 0;
    uint8_t equip_slot = 0;
    std::string text;
    uint8_t race = 0;
    uint8_t cls = 0;
    uint8_t spell_id = 0;
    uint8_t cheat_id = 0;

    static Command move(uint16_t x, uint16_t y) {
        Command c;
        c.type = MsgType::MOVE;
        c.pos_x = x;
        c.pos_y = y;
        return c;
    }

    static Command login(const std::string& username) {
        Command c;
        c.type = MsgType::LOGIN;
        c.text = username;
        return c;
    }

    static Command register_player(const std::string& username, uint8_t race, uint8_t cls) {
        Command c;
        c.type = MsgType::REGISTER;
        c.text = username;
        c.race = race;
        c.cls = cls;
        return c;
    }

    static Command attack(uint16_t target_id) {
        Command c;
        c.type = MsgType::ATTACK;
        c.target_id = target_id;
        return c;
    }

    static Command chat(const std::string& msg) {
        Command c;
        c.type = MsgType::CHAT_COMMAND;
        c.text = msg;
        return c;
    }

    static Command equip(uint8_t slot) {
        Command c;
        c.type = MsgType::EQUIP_ITEM;
        c.slot = slot;
        return c;
    }

    static Command unequip(EquipSlot slot) {
        Command c;
        c.type = MsgType::UNEQUIP_ITEM;
        c.equip_slot = static_cast<uint8_t>(slot);
        return c;
    }

    static Command drop(uint8_t slot) {
        Command c;
        c.type = MsgType::DROP_ITEM;
        c.slot = slot;
        return c;
    }

    static Command move_item(uint8_t from_slot, uint8_t to_slot) {
        Command c;
        c.type = MsgType::MOVE_ITEM;
        c.slot = from_slot;
        c.to_slot = to_slot;
        return c;
    }

    static Command pick_item() {
        Command c;
        c.type = MsgType::PICK_ITEM;
        return c;
    }

    static Command use_item(uint8_t slot) {
        Command c;
        c.type = MsgType::USE_ITEM;
        c.slot = slot;
        return c;
    }

    static Command meditate() {
        Command c;
        c.type = MsgType::MEDITATE;
        return c;
    }

    static Command resurrect() {
        Command c;
        c.type = MsgType::RESURRECT;
        return c;
    }

    static Command npc_interact(uint16_t npc_id) {
        Command c;
        c.type = MsgType::NPC_INTERACT;
        c.target_id = npc_id;
        return c;
    }

    static Command logout() {
        Command c;
        c.type = MsgType::LOGOUT;
        return c;
    }

    static Command cast_spell(uint16_t target_id, uint8_t spell_id) {
        Command c;
        c.type = MsgType::CAST_SPELL;
        c.target_id = target_id;
        c.spell_id = spell_id;
        return c;
    }

    static Command cheat(CheatId id) {
        Command c;
        c.type = MsgType::CHEAT;
        c.cheat_id = static_cast<uint8_t>(id);
        return c;
    }
};