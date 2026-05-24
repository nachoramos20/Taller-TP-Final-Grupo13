#pragma once

#include <cstdint>
#include <string>
#include "../../common/protocol/protocol.h"

struct Command {
    MsgType type;

    uint16_t    pos_x     = 0;
    uint16_t    pos_y     = 0;
    uint16_t    target_id = 0;
    uint8_t     slot      = 0;
    std::string text;
    uint8_t     race      = 0;
    uint8_t     cls       = 0;

    static Command move(uint16_t x, uint16_t y) {
        Command c;
        c.type  = MsgType::MOVE;
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

    static Command register_player(const std::string& username,
                                   uint8_t race, uint8_t cls) {
        Command c;
        c.type = MsgType::REGISTER;
        c.text = username;
        c.race = race;
        c.cls  = cls;
        return c;
    }

    static Command attack(uint16_t target_id) {
        Command c;
        c.type      = MsgType::ATTACK;
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

    static Command drop(uint8_t slot) {
        Command c;
        c.type = MsgType::DROP_ITEM;
        c.slot = slot;
        return c;
    }

    static Command pick_item() {
        Command c;
        c.type = MsgType::PICK_ITEM;
        return c;
    }

    static Command logout() {
        Command c;
        c.type = MsgType::LOGOUT;
        return c;
    }
};