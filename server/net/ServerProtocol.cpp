#include "ServerProtocol.h"

#include <arpa/inet.h>
#include <utility>
#include <vector>
#include <stdexcept>
#include "../../common/protocol/protocol.h"
#include <iostream>

ServerProtocol::ServerProtocol(Socket&& socket) : socket(std::move(socket)) {}

std::shared_ptr<ServerCommand> ServerProtocol::receive_command(uint16_t client_id) {
    uint8_t code;
    int n = socket.recvall(&code, 1);
    if (n == 0) return nullptr;

    switch (static_cast<MsgType>(code)) {
        case MsgType::MOVE:         return receive_move_command(client_id);
        case MsgType::ATTACK:       return receive_attack(client_id);
        case MsgType::EQUIP_ITEM:   return receive_equip(client_id);
        case MsgType::UNEQUIP_ITEM: return receive_unequip(client_id);
        case MsgType::DROP_ITEM:    return receive_drop(client_id);
        case MsgType::PICK_ITEM:    return receive_pick(client_id);
        case MsgType::USE_ITEM:     return receive_use(client_id);
        case MsgType::MEDITATE:     return receive_meditate(client_id);
        case MsgType::RESURRECT:    return receive_resurrect(client_id);
        case MsgType::LOGOUT:       return receive_logout(client_id);
        case MsgType::CHAT_COMMAND: return receive_chat_command(client_id);
        case MsgType::NPC_INTERACT: return receive_npc_interact(client_id);
        case MsgType::CAST_SPELL:   return receive_cast_spell(client_id);

        default: return nullptr;
    }
}

std::shared_ptr<MoveCommand> ServerProtocol::receive_move_command(uint16_t client_id) {
    uint16_t xn, yn;
    socket.recvall(&xn, sizeof(xn));
    socket.recvall(&yn, sizeof(yn));
    return std::make_shared<MoveCommand>(client_id, ntohs(xn), ntohs(yn));
}

std::shared_ptr<AttackCommand> ServerProtocol::receive_attack(uint16_t client_id) {
    uint16_t tn; socket.recvall(&tn, sizeof(tn));
    return std::make_shared<AttackCommand>(client_id, ntohs(tn));
}

std::shared_ptr<CastSpellCommand> ServerProtocol::receive_cast_spell(uint16_t client_id) {
    uint16_t tn; socket.recvall(&tn, sizeof(tn));
    uint8_t spell_id = recv_uint8();
    return std::make_shared<CastSpellCommand>(client_id, ntohs(tn), spell_id);
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
    std::string text = recv_str8();
    return std::make_shared<ChatCommand>(client_id, std::move(text));
}

std::shared_ptr<NpcInteractCommand> ServerProtocol::receive_npc_interact(uint16_t client_id) {
    uint16_t tn; socket.recvall(&tn, sizeof(tn));
    return std::make_shared<NpcInteractCommand>(client_id, ntohs(tn));
}

void ServerProtocol::send_login_ok(uint16_t entity_id) {
    send_uint8((uint8_t)MsgType::LOGIN_OK);
    send_uint16(entity_id);
}

void ServerProtocol::send_login_error(const std::string& msg) {
    send_uint8((uint8_t)MsgType::LOGIN_ERROR);
    send_str8(msg);
}

void ServerProtocol::send_mapa(const MapaDTO& mapa) {
    send_uint8((uint8_t)MsgType::MAPA);
    if (mapa.width == 0 && mapa.height == 0 && mapa.tiles.empty()) return;
    send_uint16(mapa.width);
    send_uint16(mapa.height);
    send_uint32((uint32_t)mapa.tiles.size());
    for (const TileDTO& t : mapa.tiles) {
        send_uint16(t.floor_id);
        send_uint16(t.object_id);
        send_uint16(t.object_superior_id);
    }
}

void ServerProtocol::send_snapshot(const SnapshotDTO& snap) {
    send_uint8((uint8_t)MsgType::SNAPSHOT);
    send_uint32(snap.tick);
    send_uint16(snap.self_entity_id);
    send_uint16(snap.hp); send_uint16(snap.max_hp);
    send_uint16(snap.mp); send_uint16(snap.max_mp);
    send_uint32(snap.exp); send_uint8(snap.level);
    send_uint8(snap.cls);
    send_uint32(snap.gold);
    send_uint8(snap.is_ghost); send_uint8(snap.meditating);

    send_uint8((uint8_t)SnapshotDTO::INVENTORY_SIZE);
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++) send_uint8(snap.inventory[i]);
    send_uint8(snap.equipped_wpn);
    send_uint8(snap.equipped_arm);
    send_uint8(snap.equipped_helm);
    send_uint8(snap.equipped_shld);

    if (snap.entities) {
        send_uint8((uint8_t)snap.entities->size());
        for (const auto& e : *snap.entities) {
            send_uint16(e.entity_id);
            send_uint8(e.entity_type);
            send_str8(e.username);
            send_uint16(e.pos_x); send_uint16(e.pos_y);
            send_uint8(e.direction); send_uint8(e.sprite_id);
            send_uint8(e.is_ghost); send_uint8(e.hp_pct);
        }
    } else send_uint8(0);

    if (snap.messages) {
        send_uint8((uint8_t)snap.messages->size());
        for (const auto& m : *snap.messages) {
            send_uint8(m.msg_type);
            send_str8(m.text);
        }
    } else send_uint8(0);
}

MsgType ServerProtocol::receive_handshake() {
    uint8_t code;
    int n = socket.recvall(&code, 1);
    if (n == 0) throw std::runtime_error("Handshake: peer closed");
    auto m = static_cast<MsgType>(code);
    if (m != MsgType::LOGIN && m != MsgType::REGISTER)
        throw std::runtime_error("Handshake: invalid opcode");
    return m;
}

void ServerProtocol::handshake_login(std::string& username) {
    uint8_t len; socket.recvall(&len, 1);
    std::vector<char> buf(len);
    socket.recvall(buf.data(), len);
    username.assign(buf.data(), len);
}

void ServerProtocol::handshake_register(std::string& username, uint8_t& race, uint8_t& cls) {
    handshake_login(username);
    socket.recvall(&race, 1);
    socket.recvall(&cls, 1);
}

uint8_t  ServerProtocol::recv_uint8()  { uint8_t v; socket.recvall(&v,1); return v; }
uint16_t ServerProtocol::recv_uint16() { uint16_t v; socket.recvall(&v,2); return ntohs(v); }
std::string ServerProtocol::recv_str8() {
    uint8_t len = recv_uint8();
    std::vector<char> buf(len);
    socket.recvall(buf.data(), len);
    return std::string(buf.data(), len);
}

void ServerProtocol::send_uint8(uint8_t v)  { socket.sendall(&v, 1); }
void ServerProtocol::send_uint16(uint16_t v){ uint16_t n=htons(v); socket.sendall(&n,2); }
void ServerProtocol::send_uint32(uint32_t v){ uint32_t n=htonl(v); socket.sendall(&n,4); }
void ServerProtocol::send_str8(const std::string& s){
    send_uint8((uint8_t)s.size());
    socket.sendall(s.data(), s.size());
}

void ServerProtocol::shutdown(int how) { socket.shutdown(how); }
