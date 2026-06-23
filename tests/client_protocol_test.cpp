#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "../client/net/ClientProtocol.h"
#include "../common/protocol/protocol.h"
#include "../common/protocol/dtos.h"

using ::testing::_;
using ::testing::NotNull;

namespace {

struct TestSockets {
    int fd_server;
    int fd_client;

    TestSockets() {
        int fds[2];
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
            throw std::runtime_error("socketpair failed");
        fd_server = fds[0];
        fd_client = fds[1];
    }

    ~TestSockets() {
        ::close(fd_server);
        ::close(fd_client);
    }

    Socket make_server_socket() { return Socket(fd_server); }
    Socket make_client_socket() { return Socket(fd_client); }
};

// helpers

void send_bytes(int fd, const void* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = ::write(fd, static_cast<const char*>(data) + total, len - total);
        if (n <= 0)
            throw std::runtime_error("error al enviar bytes");
        total += n;
    }
}

std::vector<uint8_t> recv_bytes(int fd, size_t len) {
    std::vector<uint8_t> buf(len);
    size_t total = 0;
    while (total < len) {
        ssize_t n = ::read(fd, buf.data() + total, len - total);
        if (n <= 0)
            throw std::runtime_error("error al recibir bytes");
        total += n;
    }
    return buf;
}

void send_uint8(int fd, uint8_t val) {
    send_bytes(fd, &val, 1);
}

void send_uint16(int fd, uint16_t val) {
    val = htons(val);
    send_bytes(fd, &val, sizeof(val));
}

void send_uint32(int fd, uint32_t val) {
    val = htonl(val);
    send_bytes(fd, &val, sizeof(val));
}

void send_string(int fd, const std::string& s) {
    uint8_t len = static_cast<uint8_t>(s.size());
    send_bytes(fd, &len, 1);
    send_bytes(fd, s.data(), s.size());
}

// Test de envio 

TEST(ClientProtocolSendTest, SendLogin) {
    TestSockets sockets;
    std::string username = "TestPlayer";
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_login(username);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t) + username.size());

    size_t pos = 0;
    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::LOGIN)); pos += sizeof(uint8_t);
    uint8_t len = buffer[pos]; pos += sizeof(uint8_t);
    EXPECT_EQ(len, username.size());
    std::string recibido(buffer.begin() + pos, buffer.begin() + pos + len);
    EXPECT_EQ(recibido, username);
}

TEST(ClientProtocolSendTest, SendRegister) {
    TestSockets sockets;
    std::string username = "NewUser";
    uint8_t race_id = static_cast<uint8_t>(Race::ELF);
    uint8_t class_id = static_cast<uint8_t>(Class::MAGE);
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_register(username, Race::ELF, Class::MAGE);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t) + username.size() + sizeof(uint8_t) + sizeof(uint8_t));

    size_t pos = 0;
    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::REGISTER)); pos += sizeof(uint8_t);
    uint8_t len = buffer[pos]; pos += sizeof(uint8_t);
    EXPECT_EQ(len, username.size());
    std::string recibido(buffer.begin() + pos, buffer.begin() + pos + len); pos += len;
    EXPECT_EQ(recibido, username);
    EXPECT_EQ(buffer[pos], race_id); pos += sizeof(uint8_t);
    EXPECT_EQ(buffer[pos], class_id);
}

TEST(ClientProtocolSendTest, SendMove) {
    TestSockets sockets;
    uint16_t x = 100;
    uint16_t y = 200;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_move(x, y);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t));

    size_t pos = 0;
    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::MOVE)); pos += sizeof(uint8_t);
    uint16_t x_recibida;
    memcpy(&x_recibida, &buffer[pos], sizeof(uint16_t)); pos += sizeof(uint16_t);
    EXPECT_EQ(ntohs(x_recibida), x);
    uint16_t y_recibida;
    memcpy(&y_recibida, &buffer[pos], sizeof(uint16_t));
    EXPECT_EQ(ntohs(y_recibida), y);
}

TEST(ClientProtocolSendTest, SendAttack) {
    TestSockets sockets;
    uint16_t target_id = 7;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_attack(target_id);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint16_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::ATTACK));
    uint16_t target_recibida;
    memcpy(&target_recibida, &buffer[sizeof(uint8_t)], sizeof(uint16_t));
    EXPECT_EQ(ntohs(target_recibida), target_id);
}

TEST(ClientProtocolSendTest, SendChatCommand) {
    TestSockets sockets;
    std::string text = "/meditar";
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_chat_command(text);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t) + text.size());

    size_t pos = 0;
    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::CHAT_COMMAND)); pos += sizeof(uint8_t);
    uint8_t len = buffer[pos]; pos += sizeof(uint8_t);
    EXPECT_EQ(len, text.size());
    std::string recibido(buffer.begin() + pos, buffer.begin() + pos + len);
    EXPECT_EQ(recibido, text);
}

TEST(ClientProtocolSendTest, SendChatCommandEmpty) {
    TestSockets sockets;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_chat_command("");
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::CHAT_COMMAND));
    EXPECT_EQ(buffer[1], 0);
}

TEST(ClientProtocolSendTest, SendEquipItem) {
    TestSockets sockets;
    uint8_t slot = 3;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_equip_item(slot);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::EQUIP_ITEM));
    EXPECT_EQ(buffer[1], slot);
}

TEST(ClientProtocolSendTest, SendUnequipItem) {
    TestSockets sockets;
    uint8_t slot = static_cast<uint8_t>(EquipSlot::HELMET);
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_unequip_item(EquipSlot::HELMET);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::UNEQUIP_ITEM));
    EXPECT_EQ(buffer[1], slot);
}

TEST(ClientProtocolSendTest, SendMoveItem) {
    TestSockets sockets;
    uint8_t from = 2;
    uint8_t to = 5;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_move_item(from, to);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t));

    size_t pos = 0;
    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::MOVE_ITEM)); pos += sizeof(uint8_t);
    EXPECT_EQ(buffer[pos], from); pos += sizeof(uint8_t);
    EXPECT_EQ(buffer[pos], to);
}

TEST(ClientProtocolSendTest, SendDropItem) {
    TestSockets sockets;
    uint8_t slot = 5;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_drop_item(slot);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::DROP_ITEM));
    EXPECT_EQ(buffer[1], slot);
}

TEST(ClientProtocolSendTest, SendPickItem) {
    TestSockets sockets;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_pick_item();
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server, sizeof(uint8_t));
    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::PICK_ITEM));
}

TEST(ClientProtocolSendTest, SendUseItem) {
    TestSockets sockets;
    uint8_t slot = 1;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_use_item(slot);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::USE_ITEM));
    EXPECT_EQ(buffer[1], slot);
}

TEST(ClientProtocolSendTest, SendMeditate) {
    TestSockets sockets;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_meditate();
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server, sizeof(uint8_t));
    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::MEDITATE));
}

TEST(ClientProtocolSendTest, SendResurrect) {
    TestSockets sockets;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_resurrect();
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server, sizeof(uint8_t));
    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::RESURRECT));
}

TEST(ClientProtocolSendTest, SendLogout) {
    TestSockets sockets;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_logout();
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server, sizeof(uint8_t));
    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::LOGOUT));
}

TEST(ClientProtocolSendTest, SendNpcInteract) {
    TestSockets sockets;
    uint16_t npc_id = 3;
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_npc_interact(npc_id);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint16_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::NPC_INTERACT));
    uint16_t npc_recibida;
    memcpy(&npc_recibida, &buffer[sizeof(uint8_t)], sizeof(uint16_t));
    EXPECT_EQ(ntohs(npc_recibida), npc_id);
}

TEST(ClientProtocolSendTest, SendCastSpell) {
    TestSockets sockets;
    uint16_t target_id = 10;
    uint8_t spell_id = static_cast<uint8_t>(SpellId::BURST);
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_cast_spell(target_id, spell_id);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t));

    size_t pos = 0;
    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::CAST_SPELL)); pos += sizeof(uint8_t);
    uint16_t target_recibida;
    memcpy(&target_recibida, &buffer[pos], sizeof(uint16_t)); pos += sizeof(uint16_t);
    EXPECT_EQ(ntohs(target_recibida), target_id);
    EXPECT_EQ(buffer[pos], spell_id);
}

TEST(ClientProtocolSendTest, SendCheat) {
    TestSockets sockets;
    uint8_t cheat_id = static_cast<uint8_t>(CheatId::INFINITE_HP);
    {
        Socket client_socket = sockets.make_client_socket();
        ClientProtocol protocolo_cliente(std::move(client_socket));
        protocolo_cliente.send_cheat(cheat_id);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_server,
        sizeof(uint8_t) + sizeof(uint8_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::CHEAT));
    EXPECT_EQ(buffer[1], cheat_id);
}

// Tests recibir

TEST(ClientProtocolRecvTest, RecvOpcode) {
    TestSockets sockets;
    uint8_t opcode = static_cast<uint8_t>(MsgType::LOGIN_OK);
    send_bytes(sockets.fd_server, &opcode, 1);

    Socket client_socket = sockets.make_client_socket();
    ClientProtocol protocolo_cliente(std::move(client_socket));
    EXPECT_EQ(protocolo_cliente.recv_opcode(), MsgType::LOGIN_OK);
}

TEST(ClientProtocolRecvTest, RecvLoginOk) {
    TestSockets sockets;
    uint8_t opcode = static_cast<uint8_t>(MsgType::LOGIN_OK);
    uint16_t entity_id = htons(1234);
    send_bytes(sockets.fd_server, &opcode, 1);
    send_bytes(sockets.fd_server, &entity_id, sizeof(entity_id));

    Socket client_socket = sockets.make_client_socket();
    ClientProtocol protocolo_cliente(std::move(client_socket));
    EXPECT_EQ(protocolo_cliente.recv_opcode(), MsgType::LOGIN_OK);
    EXPECT_EQ(protocolo_cliente.recv_login_ok(), 1234);
}

TEST(ClientProtocolRecvTest, RecvLoginError) {
    TestSockets sockets;
    std::string error_msg = "Bad credentials";
    uint8_t opcode = static_cast<uint8_t>(MsgType::LOGIN_ERROR);
    send_bytes(sockets.fd_server, &opcode, 1);
    send_string(sockets.fd_server, error_msg);

    Socket client_socket = sockets.make_client_socket();
    ClientProtocol protocolo_cliente(std::move(client_socket));
    EXPECT_EQ(protocolo_cliente.recv_opcode(), MsgType::LOGIN_ERROR);
    EXPECT_EQ(protocolo_cliente.recv_login_error(), error_msg);
}

TEST(ClientProtocolRecvTest, RecvLoginErrorEmpty) {
    TestSockets sockets;
    uint8_t opcode = static_cast<uint8_t>(MsgType::LOGIN_ERROR);
    send_bytes(sockets.fd_server, &opcode, 1);
    send_string(sockets.fd_server, "");

    Socket client_socket = sockets.make_client_socket();
    ClientProtocol protocolo_cliente(std::move(client_socket));
    EXPECT_EQ(protocolo_cliente.recv_opcode(), MsgType::LOGIN_ERROR);
    EXPECT_EQ(protocolo_cliente.recv_login_error(), "");
}

TEST(ClientProtocolRecvTest, RecvMap) {
    TestSockets sockets;
    uint16_t width = htons(2);
    uint16_t height = htons(2);
    uint32_t num_tiles = htonl(4);
    uint16_t floor1 = htons(10), obj1 = htons(20), obj_sup1 = htons(30);
    uint16_t floor2 = htons(11), obj2 = htons(21), obj_sup2 = htons(31);
    uint16_t floor3 = htons(12), obj3 = htons(22), obj_sup3 = htons(32);
    uint16_t floor4 = htons(13), obj4 = htons(23), obj_sup4 = htons(33);

    send_bytes(sockets.fd_server, &width, sizeof(width));
    send_bytes(sockets.fd_server, &height, sizeof(height));
    send_bytes(sockets.fd_server, &num_tiles, sizeof(num_tiles));
    send_bytes(sockets.fd_server, &floor1, sizeof(floor1)); send_bytes(sockets.fd_server, &obj1, sizeof(obj1)); send_bytes(sockets.fd_server, &obj_sup1, sizeof(obj_sup1));
    send_bytes(sockets.fd_server, &floor2, sizeof(floor2)); send_bytes(sockets.fd_server, &obj2, sizeof(obj2)); send_bytes(sockets.fd_server, &obj_sup2, sizeof(obj_sup2));
    send_bytes(sockets.fd_server, &floor3, sizeof(floor3)); send_bytes(sockets.fd_server, &obj3, sizeof(obj3)); send_bytes(sockets.fd_server, &obj_sup3, sizeof(obj_sup3));
    send_bytes(sockets.fd_server, &floor4, sizeof(floor4)); send_bytes(sockets.fd_server, &obj4, sizeof(obj4)); send_bytes(sockets.fd_server, &obj_sup4, sizeof(obj_sup4));

    Socket client_socket = sockets.make_client_socket();
    ClientProtocol protocolo_cliente(std::move(client_socket));
    MapaDTO mapa = protocolo_cliente.recv_map();
    EXPECT_EQ(mapa.width, 2);
    EXPECT_EQ(mapa.height, 2);
    EXPECT_EQ(mapa.tiles.size(), 4);
    EXPECT_EQ(mapa.tiles[0].floor_id, 10);
    EXPECT_EQ(mapa.tiles[0].object_id, 20);
    EXPECT_EQ(mapa.tiles[0].object_superior_id, 30);
    EXPECT_EQ(mapa.tiles[3].floor_id, 13);
    EXPECT_EQ(mapa.tiles[3].object_id, 23);
    EXPECT_EQ(mapa.tiles[3].object_superior_id, 33);
}

TEST(ClientProtocolRecvTest, RecvSnapshot) {
    TestSockets sockets;

    // Enviar datos del snapshot manualmente
    send_uint32(sockets.fd_server, 42);     // tick
    send_uint16(sockets.fd_server, 1);      // self_entity_id
    send_uint16(sockets.fd_server, 100);    // hp
    send_uint16(sockets.fd_server, 200);    // max_hp
    send_uint16(sockets.fd_server, 50);     // mp
    send_uint16(sockets.fd_server, 150);    // max_mp
    send_uint32(sockets.fd_server, 9999);   // exp
    send_uint8(sockets.fd_server, 5);       // level
    send_uint8(sockets.fd_server, static_cast<uint8_t>(Class::WARRIOR));  // character_class
    send_uint32(sockets.fd_server, 12345);  // gold
    send_uint8(sockets.fd_server, 0);       // is_ghost
    send_uint8(sockets.fd_server, 1);       // meditating

    // Inventory
    send_uint8(sockets.fd_server, SnapshotDTO::INVENTORY_SIZE);
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
        send_uint8(sockets.fd_server, i + 1);
    send_uint8(sockets.fd_server, 1);  // equipped_weapon
    send_uint8(sockets.fd_server, 2);  // equipped_armor
    send_uint8(sockets.fd_server, 3);  // equipped_helmet
    send_uint8(sockets.fd_server, 4);  // equipped_shield

    // Entities: 1 entity
    send_uint8(sockets.fd_server, 1);
    send_uint16(sockets.fd_server, 10);       // entity_id
    send_uint8(sockets.fd_server, static_cast<uint8_t>(EntityType::PLAYER));  // entity_type
    send_string(sockets.fd_server, "Player1");
    send_uint16(sockets.fd_server, 5);         // pos_x
    send_uint16(sockets.fd_server, 6);         // pos_y
    send_uint8(sockets.fd_server, 1);          // direction
    send_uint8(sockets.fd_server, 2);          // sprite_id
    send_uint8(sockets.fd_server, 0);          // is_ghost
    send_uint8(sockets.fd_server, 80);         // hp_pct
    send_uint8(sockets.fd_server, 1);          // equipped_weapon
    send_uint8(sockets.fd_server, 0);          // equipped_armor
    send_uint8(sockets.fd_server, 0);          // equipped_helmet
    send_uint8(sockets.fd_server, 0);          // equipped_shield
    send_uint8(sockets.fd_server, 0);          // level

    // Messages: 1 message
    send_uint8(sockets.fd_server, 1);
    send_uint8(sockets.fd_server, 0);          // msg_type
    send_string(sockets.fd_server, "Welcome!");

    // Spell events: 1 spell event
    send_uint8(sockets.fd_server, 1);
    send_uint16(sockets.fd_server, 10);        // caster_id
    send_uint8(sockets.fd_server, 5);          // spell_id
    send_uint16(sockets.fd_server, 20);        // target_x
    send_uint16(sockets.fd_server, 30);        // target_y
    send_uint8(sockets.fd_server, 1);          // is_magic_projectile

    Socket client_socket = sockets.make_client_socket();
    ClientProtocol protocolo_cliente(std::move(client_socket));
    SnapshotDTO snap = protocolo_cliente.recv_snapshot();

    EXPECT_EQ(snap.tick, 42);
    EXPECT_EQ(snap.self_entity_id, 1);
    EXPECT_EQ(snap.hp, 100);
    EXPECT_EQ(snap.max_hp, 200);
    EXPECT_EQ(snap.mp, 50);
    EXPECT_EQ(snap.max_mp, 150);
    EXPECT_EQ(snap.exp, 9999);
    EXPECT_EQ(snap.level, 5);
    EXPECT_EQ(snap.character_class, static_cast<uint8_t>(Class::WARRIOR));
    EXPECT_EQ(snap.gold, 12345);
    EXPECT_EQ(snap.is_ghost, 0);
    EXPECT_EQ(snap.meditating, 1);

    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
        EXPECT_EQ(snap.inventory[i], i + 1);
    EXPECT_EQ(snap.equipped_weapon, 1);
    EXPECT_EQ(snap.equipped_armor, 2);
    EXPECT_EQ(snap.equipped_helmet, 3);
    EXPECT_EQ(snap.equipped_shield, 4);

    ASSERT_NE(snap.entities, nullptr);
    ASSERT_EQ(snap.entities->size(), 1);
    EXPECT_EQ((*snap.entities)[0].entity_id, 10);
    EXPECT_EQ((*snap.entities)[0].entity_type, static_cast<uint8_t>(EntityType::PLAYER));
    EXPECT_EQ((*snap.entities)[0].username, "Player1");
    EXPECT_EQ((*snap.entities)[0].pos_x, 5);
    EXPECT_EQ((*snap.entities)[0].pos_y, 6);
    EXPECT_EQ((*snap.entities)[0].direction, 1);
    EXPECT_EQ((*snap.entities)[0].sprite_id, 2);
    EXPECT_EQ((*snap.entities)[0].is_ghost, 0);
    EXPECT_EQ((*snap.entities)[0].hp_pct, 80);
    EXPECT_EQ((*snap.entities)[0].equipped_weapon, 1);
    EXPECT_EQ((*snap.entities)[0].equipped_armor, 0);
    EXPECT_EQ((*snap.entities)[0].equipped_helmet, 0);
    EXPECT_EQ((*snap.entities)[0].equipped_shield, 0);
    EXPECT_EQ((*snap.entities)[0].level, 0);

    ASSERT_NE(snap.messages, nullptr);
    ASSERT_EQ(snap.messages->size(), 1);
    EXPECT_EQ((*snap.messages)[0].msg_type, 0);
    EXPECT_EQ((*snap.messages)[0].text, "Welcome!");

    ASSERT_NE(snap.spell_events, nullptr);
    ASSERT_EQ(snap.spell_events->size(), 1);
    EXPECT_EQ((*snap.spell_events)[0].caster_id, 10);
    EXPECT_EQ((*snap.spell_events)[0].spell_id, 5);
    EXPECT_EQ((*snap.spell_events)[0].target_x, 20);
    EXPECT_EQ((*snap.spell_events)[0].target_y, 30);
    EXPECT_EQ((*snap.spell_events)[0].is_magic_projectile, true);
}

TEST(ClientProtocolRecvTest, RecvSnapshotNoEntitiesNoMessages) {
    TestSockets sockets;

    send_uint32(sockets.fd_server, 1);      // tick
    send_uint16(sockets.fd_server, 42);     // self_entity_id
    send_uint16(sockets.fd_server, 100);    // hp
    send_uint16(sockets.fd_server, 100);    // max_hp
    send_uint16(sockets.fd_server, 50);     // mp
    send_uint16(sockets.fd_server, 50);     // max_mp
    send_uint32(sockets.fd_server, 0);      // exp
    send_uint8(sockets.fd_server, 1);       // level
    send_uint8(sockets.fd_server, static_cast<uint8_t>(Class::MAGE));  // character_class
    send_uint32(sockets.fd_server, 0);      // gold
    send_uint8(sockets.fd_server, 0);       // is_ghost
    send_uint8(sockets.fd_server, 0);       // meditating

    // Inventory
    send_uint8(sockets.fd_server, SnapshotDTO::INVENTORY_SIZE);
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
        send_uint8(sockets.fd_server, 0);
    send_uint8(sockets.fd_server, 0);  // equipped_weapon
    send_uint8(sockets.fd_server, 0);  // equipped_armor
    send_uint8(sockets.fd_server, 0);  // equipped_helmet
    send_uint8(sockets.fd_server, 0);  // equipped_shield

    // Entities: 0
    send_uint8(sockets.fd_server, 0);

    // Messages: 0
    send_uint8(sockets.fd_server, 0);

    // Spell events: 0
    send_uint8(sockets.fd_server, 0);

    Socket client_socket = sockets.make_client_socket();
    ClientProtocol protocolo_cliente(std::move(client_socket));
    SnapshotDTO snap = protocolo_cliente.recv_snapshot();

    EXPECT_EQ(snap.tick, 1);
    EXPECT_EQ(snap.self_entity_id, 42);

    ASSERT_NE(snap.entities, nullptr);
    EXPECT_EQ(snap.entities->size(), 0);
    ASSERT_NE(snap.messages, nullptr);
    EXPECT_EQ(snap.messages->size(), 0);
    ASSERT_NE(snap.spell_events, nullptr);
    EXPECT_EQ(snap.spell_events->size(), 0);
}

} // anonymous namespace