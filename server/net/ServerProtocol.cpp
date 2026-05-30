#include "ServerProtocol.h"

#include <arpa/inet.h>
#include <utility>
#include <vector>
#include "../../common/protocol/protocol.h"

ServerProtocol::ServerProtocol(Socket&& socket)
    : socket(std::move(socket)) {}

#define CODIGO_COMANDO_LOGIN 0x01
#define CODIGO_COMANDO_MOVERSE 0x03


std::shared_ptr<ServerCommand> ServerProtocol::receive_command(uint16_t client_id) {


    
    uint8_t codigo_inicio;
    int bytes_recibidos = this->socket.recvall(&codigo_inicio, sizeof(codigo_inicio));
    if (bytes_recibidos == 0) {
        return nullptr;
    }

    switch (static_cast<MsgType>(codigo_inicio)) {
        case MsgType::LOGIN:
            return this->receive_login_command(client_id);
        case MsgType::MOVE:
            return this->receive_move_command(client_id);
        default:
            return nullptr;
    }

    //TODO Implementar un MAP en vez de un switch case

}

std::shared_ptr<LoginCommand> ServerProtocol::receive_login_command(uint16_t client_id) {
    uint8_t username_len;
    socket.recvall(&username_len, sizeof(username_len));
    std::vector<char> username_buf(username_len);
    socket.recvall(username_buf.data(), username_len);


    return std::make_shared<LoginCommand>(client_id, std::string(username_buf.data(), username_len));
}

std::shared_ptr<MoveCommand> ServerProtocol::receive_move_command(uint16_t client_id) {
    uint16_t pos_x_network;
    uint16_t pos_y_network;
    socket.recvall(&pos_x_network, sizeof(pos_x_network));
    socket.recvall(&pos_y_network, sizeof(pos_y_network));
    uint16_t pos_x = ntohs(pos_x_network);
    uint16_t pos_y = ntohs(pos_y_network);

    return std::make_shared<MoveCommand>(client_id, pos_x, pos_y);
}

void ServerProtocol::send_snapshot(const SnapshotDTO& snap) {
        send_uint8(static_cast<uint8_t>(MsgType::SNAPSHOT));
        send_uint32(snap.tick);
        send_uint16(snap.self_entity_id);
        send_uint16(snap.hp);
        send_uint16(snap.max_hp);
        send_uint16(snap.mp);
        send_uint16(snap.max_mp);
        send_uint32(snap.exp);
        send_uint8(snap.level);
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

        if (snap.entities) {
            const std::vector<EntityDTO>& entities = *snap.entities;
            send_uint8(static_cast<uint8_t>(entities.size()));
            for (std::vector<EntityDTO>::const_iterator it = entities.begin();
                 it != entities.end();
                 ++it) {
                const EntityDTO& e = *it;
                send_uint16(e.entity_id);
                send_uint8(e.entity_type);
                send_uint16(e.pos_x);
                send_uint16(e.pos_y);
                send_uint8(e.direction);
                send_uint8(e.sprite_id);
                send_uint8(e.is_ghost);
                send_uint8(e.hp_pct);
            }
        } else {
            send_uint8(0);
        }

        if (snap.messages) {
            const std::vector<ChatMessageDTO>& messages = *snap.messages;
            send_uint8(static_cast<uint8_t>(messages.size()));
            for (std::vector<ChatMessageDTO>::const_iterator it = messages.begin();
                 it != messages.end();
                 ++it) {
                const ChatMessageDTO& m = *it;
                send_uint8(m.msg_type);
                send_str8(m.text);
            }
        } else {
            send_uint8(0);
        }
    }

void ServerProtocol::send_uint8(uint8_t value) {
    socket.sendall(&value, sizeof(value));
}

void ServerProtocol::send_uint16(uint16_t value) {
    uint16_t net_value = htons(value);
    socket.sendall(&net_value, sizeof(net_value));
}

void ServerProtocol::send_uint32(uint32_t value) {
    uint32_t net_value = htonl(value);
    socket.sendall(&net_value, sizeof(net_value));
}

void ServerProtocol::send_str8(const std::string& s) {
    uint8_t len = static_cast<uint8_t>(s.size());
    send_uint8(len);
    socket.sendall(s.data(), len);
}

void ServerProtocol::shutdown(int how) {
    this->socket.shutdown(how);
}

