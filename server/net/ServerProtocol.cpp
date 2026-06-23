#include "ServerProtocol.h"

#include <utility>
#include <stdexcept>

ServerProtocol::ServerProtocol(Socket&& socket) : Protocol(std::move(socket)) {}

std::shared_ptr<ServerCommand> ServerProtocol::receive_command(uint16_t client_id) {
    uint8_t code;
    int n = _socket.recvall(&code, 1);
    if (n == 0) return nullptr;

    switch (static_cast<MsgType>(code)) {
        case MsgType::MOVE:         return receive_move_command(client_id);
        case MsgType::ATTACK:       return receive_attack(client_id);
        case MsgType::EQUIP_ITEM:   return receive_equip(client_id);
        case MsgType::UNEQUIP_ITEM: return receive_unequip(client_id);
        case MsgType::DROP_ITEM:    return receive_drop(client_id);
        case MsgType::MOVE_ITEM:    return receive_move_item(client_id);
        case MsgType::PICK_ITEM:    return receive_pick(client_id);
        case MsgType::USE_ITEM:     return receive_use(client_id);
        case MsgType::MEDITATE:     return receive_meditate(client_id);
        case MsgType::RESURRECT:    return receive_resurrect(client_id);
        case MsgType::LOGOUT:       return receive_logout(client_id);
        case MsgType::CHAT_COMMAND: return receive_chat_command(client_id);
        case MsgType::NPC_INTERACT: return receive_npc_interact(client_id);
        case MsgType::CAST_SPELL:   return receive_cast_spell(client_id);
        case MsgType::CHEAT:        return receive_cheat(client_id);

        default: return nullptr;
    }
}

std::shared_ptr<MoveCommand> ServerProtocol::receive_move_command(uint16_t client_id) {
    uint16_t x = recv_uint16();
    uint16_t y = recv_uint16();
    return std::make_shared<MoveCommand>(client_id, x, y);
}

std::shared_ptr<AttackCommand> ServerProtocol::receive_attack(uint16_t client_id) {
    uint16_t target_id = recv_uint16();
    return std::make_shared<AttackCommand>(client_id, target_id);
}

std::shared_ptr<CastSpellCommand> ServerProtocol::receive_cast_spell(uint16_t client_id) {
    uint16_t target_id = recv_uint16();
    uint8_t spell_id = recv_uint8();
    return std::make_shared<CastSpellCommand>(client_id, target_id, spell_id);
}

std::shared_ptr<EquipCommand> ServerProtocol::receive_equip(uint16_t client_id) {
    uint8_t s = recv_uint8();
    return std::make_shared<EquipCommand>(client_id, s);
}

std::shared_ptr<UnequipCommand> ServerProtocol::receive_unequip(uint16_t client_id) {
    uint8_t s = recv_uint8();
    return std::make_shared<UnequipCommand>(client_id, static_cast<EquipSlot>(s));
}

std::shared_ptr<DropCommand> ServerProtocol::receive_drop(uint16_t client_id) {
    uint8_t s = recv_uint8();
    return std::make_shared<DropCommand>(client_id, s);
}

std::shared_ptr<MoveItemCommand> ServerProtocol::receive_move_item(uint16_t client_id) {
    uint8_t from = recv_uint8();
    uint8_t to   = recv_uint8();
    return std::make_shared<MoveItemCommand>(client_id, from, to);
}

std::shared_ptr<CheatCommand> ServerProtocol::receive_cheat(uint16_t client_id) {
    uint8_t cheat_id = recv_uint8();
    return std::make_shared<CheatCommand>(client_id, cheat_id);
}

std::shared_ptr<PickCommand> ServerProtocol::receive_pick(uint16_t client_id) {
    return std::make_shared<PickCommand>(client_id);
}

std::shared_ptr<UseItemCommand> ServerProtocol::receive_use(uint16_t client_id) {
    uint8_t s = recv_uint8();
    return std::make_shared<UseItemCommand>(client_id, s);
}

std::shared_ptr<MeditateCommand> ServerProtocol::receive_meditate(uint16_t client_id) {
    return std::make_shared<MeditateCommand>(client_id);
}

std::shared_ptr<ResurrectCommand> ServerProtocol::receive_resurrect(uint16_t client_id) {
    return std::make_shared<ResurrectCommand>(client_id);
}

std::shared_ptr<LogoutCommand> ServerProtocol::receive_logout(uint16_t client_id) {
    return std::make_shared<LogoutCommand>(client_id);
}

std::shared_ptr<ChatCommand> ServerProtocol::receive_chat_command(uint16_t client_id) {
    std::string text = recv_string();
    return std::make_shared<ChatCommand>(client_id, std::move(text));
}

std::shared_ptr<NpcInteractCommand> ServerProtocol::receive_npc_interact(uint16_t client_id) {
    uint16_t npc_id = recv_uint16();
    return std::make_shared<NpcInteractCommand>(client_id, npc_id);
}

void ServerProtocol::send_login_ok(uint16_t entity_id) {
    send_uint8(static_cast<uint8_t>(MsgType::LOGIN_OK));
    send_uint16(entity_id);
}

void ServerProtocol::send_login_error(const std::string& error_message) {
    send_uint8(static_cast<uint8_t>(MsgType::LOGIN_ERROR));
    send_string(error_message);
}

void ServerProtocol::send_mapa(const MapaDTO& map_data) {
    send_uint8(static_cast<uint8_t>(MsgType::MAPA));
    if (map_data.width == 0 && map_data.height == 0 && map_data.tiles.empty()) return;
    send_uint16(map_data.width);
    send_uint16(map_data.height);
    send_uint32(static_cast<uint32_t>(map_data.tiles.size()));
    for (const TileDTO& t : map_data.tiles) {
        send_uint16(t.floor_id);
        send_uint16(t.object_id);
        send_uint16(t.object_superior_id);
    }
}

void ServerProtocol::send_snapshot(const SnapshotDTO& snapshot) {
    send_uint8(static_cast<uint8_t>(MsgType::SNAPSHOT));
    send_uint32(snapshot.tick);
    send_uint16(snapshot.self_entity_id);
    send_uint16(snapshot.hp); send_uint16(snapshot.max_hp);
    send_uint16(snapshot.mp); send_uint16(snapshot.max_mp);
    send_uint32(snapshot.exp); send_uint8(snapshot.level);
    send_uint8(snapshot.character_class);
    send_uint32(snapshot.gold);
    send_uint8(snapshot.is_ghost); send_uint8(snapshot.meditating);

    send_uint8(static_cast<uint8_t>(SnapshotDTO::INVENTORY_SIZE));
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++) send_uint8(snapshot.inventory[i]);
    send_uint8(snapshot.equipped_weapon);
    send_uint8(snapshot.equipped_armor);
    send_uint8(snapshot.equipped_helmet);
    send_uint8(snapshot.equipped_shield);

    if (snapshot.entities) {
        send_uint8(static_cast<uint8_t>(snapshot.entities->size()));
        for (const EntityDTO& e : *snapshot.entities) {
            send_uint16(e.entity_id);
            send_uint8(e.entity_type);
            send_string(e.username);
            send_uint16(e.pos_x); send_uint16(e.pos_y);
            send_uint8(e.direction); send_uint8(e.sprite_id);
            send_uint8(e.is_ghost); send_uint8(e.hp_pct);
            send_uint8(e.equipped_weapon); send_uint8(e.equipped_armor);
            send_uint8(e.equipped_helmet); send_uint8(e.equipped_shield);
            send_uint8(e.level);
        }
    } else {
        send_uint8(0);
    }

    if (snapshot.messages) {
        send_uint8(static_cast<uint8_t>(snapshot.messages->size()));
        for (const ChatMessageDTO& m : *snapshot.messages) {
            send_uint8(m.msg_type);
            send_string(m.text);
        }
    } else {
        send_uint8(0);
    }

    if (snapshot.spell_events) {
        send_uint8(static_cast<uint8_t>(snapshot.spell_events->size()));
        for (const SpellEventDTO& ev : *snapshot.spell_events) {
            send_uint16(ev.caster_id);
            send_uint8(ev.spell_id);
            send_uint16(ev.target_x);
            send_uint16(ev.target_y);
            send_uint8(ev.is_magic_projectile ? 1 : 0);
        }
    } else {
        send_uint8(0);
    }
}

MsgType ServerProtocol::receive_handshake() {
    uint8_t code;
    int n = _socket.recvall(&code, 1);
    if (n == 0) throw std::runtime_error("Handshake: peer closed");
    MsgType message_type = static_cast<MsgType>(code);
    if (message_type != MsgType::LOGIN && message_type != MsgType::REGISTER)
        throw std::runtime_error("Handshake: invalid opcode");
    return message_type;
}

void ServerProtocol::handshake_login(std::string& username) {
    username = recv_string();
}

void ServerProtocol::handshake_register(std::string& username, uint8_t& race_id, uint8_t& class_id) {
    handshake_login(username);
    race_id = recv_uint8();
    class_id = recv_uint8();
}
