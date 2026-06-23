#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common/protocol/dtos.h"
#include "../common/protocol/protocol.h"
#include "../server/game/commands/Commands.h"
#include "../server/net/ServerProtocol.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

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

void close_fd(int fd) {
    ::shutdown(fd, SHUT_RDWR);
    ::close(fd);
}

// handshake
TEST(ServerProtocolHandshakeTest, ReceiveLogin) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::LOGIN);
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    MsgType result = protocolo_server.receive_handshake();
    EXPECT_EQ(result, MsgType::LOGIN);
}

TEST(ServerProtocolHandshakeTest, ReceiveRegister) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::REGISTER);
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    MsgType result = protocolo_server.receive_handshake();
    EXPECT_EQ(result, MsgType::REGISTER);
}

TEST(ServerProtocolHandshakeTest, ClosedConnectionThrows) {
    TestSockets sockets;
    close_fd(sockets.fd_client);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    EXPECT_THROW(protocolo_server.receive_handshake(), std::runtime_error);
}

TEST(ServerProtocolHandshakeTest, InvalidOpcodeThrows) {
    TestSockets sockets;
    uint8_t code = 0xFF;
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    EXPECT_THROW(protocolo_server.receive_handshake(), std::runtime_error);
}

TEST(ServerProtocolHandshakeTest, HandshakeLogin) {
    TestSockets sockets;
    std::string username = "TestPlayer";
    uint8_t len = static_cast<uint8_t>(username.size());
    send_bytes(sockets.fd_client, &len, 1);
    send_bytes(sockets.fd_client, username.data(), username.size());

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::string username_recibido;
    protocolo_server.handshake_login(username_recibido);
    EXPECT_EQ(username_recibido, username);
}

TEST(ServerProtocolHandshakeTest, HandshakeRegister) {
    TestSockets sockets;
    std::string username = "NewUser";
    uint8_t len = static_cast<uint8_t>(username.size());
    uint8_t race = static_cast<uint8_t>(Race::ELF);
    uint8_t cls = static_cast<uint8_t>(Class::MAGE);

    send_bytes(sockets.fd_client, &len, 1);
    send_bytes(sockets.fd_client, username.data(), username.size());
    send_bytes(sockets.fd_client, &race, 1);
    send_bytes(sockets.fd_client, &cls, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::string username_recibido;
    uint8_t race_recibida = 0, cls_recibida = 0;
    protocolo_server.handshake_register(username_recibido, race_recibida, cls_recibida);
    EXPECT_EQ(username_recibido, username);
    EXPECT_EQ(race_recibida, race);
    EXPECT_EQ(cls_recibida, cls);
}

// Test comandos

TEST(ServerProtocolCmdTest, ReceiveMove) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::MOVE);
    uint16_t x = htons(100);
    uint16_t y = htons(200);
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &x, 2);
    send_bytes(sockets.fd_client, &y, 2);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<MoveCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveAttack) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::ATTACK);
    uint16_t target = htons(7);
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &target, 2);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<AttackCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveCastSpell) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::CAST_SPELL);
    uint16_t target = htons(10);
    uint8_t spell_id = static_cast<uint8_t>(SpellId::BURST);
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &target, 2);
    send_bytes(sockets.fd_client, &spell_id, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<CastSpellCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveEquip) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::EQUIP_ITEM);
    uint8_t slot = 3;
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &slot, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<EquipCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveUnequip) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::UNEQUIP_ITEM);
    uint8_t slot = static_cast<uint8_t>(EquipSlot::HELMET);
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &slot, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<UnequipCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveDrop) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::DROP_ITEM);
    uint8_t slot = 5;
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &slot, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<DropCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveMoveItem) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::MOVE_ITEM);
    uint8_t from = 2;
    uint8_t to = 5;
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &from, 1);
    send_bytes(sockets.fd_client, &to, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<MoveItemCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceivePick) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::PICK_ITEM);
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<PickCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveUse) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::USE_ITEM);
    uint8_t slot = 1;
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &slot, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<UseItemCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveMeditate) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::MEDITATE);
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<MeditateCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveResurrect) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::RESURRECT);
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<ResurrectCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveLogout) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::LOGOUT);
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<LogoutCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveChatCommand) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::CHAT_COMMAND);
    std::string text = "/meditar";
    uint8_t len = static_cast<uint8_t>(text.size());
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &len, 1);
    send_bytes(sockets.fd_client, text.data(), text.size());

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<ChatCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveChatCommandEmpty) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::CHAT_COMMAND);
    uint8_t len = 0;
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &len, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<ChatCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveChatCommandSpecialChars) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::CHAT_COMMAND);
    std::string text = "Hola! @#$% ññññ 😀";
    uint8_t len = static_cast<uint8_t>(text.size());
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &len, 1);
    send_bytes(sockets.fd_client, text.data(), text.size());

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<ChatCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveNpcInteract) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::NPC_INTERACT);
    uint16_t npc_id = htons(3);
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &npc_id, 2);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<NpcInteractCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, ReceiveCheat) {
    TestSockets sockets;
    uint8_t code = static_cast<uint8_t>(MsgType::CHEAT);
    uint8_t cheat_id = static_cast<uint8_t>(CheatId::INFINITE_HP);
    send_bytes(sockets.fd_client, &code, 1);
    send_bytes(sockets.fd_client, &cheat_id, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<CheatCommand*>(comando.get()), nullptr);
}

TEST(ServerProtocolCmdTest, UnknownCodeReturnsNullptr) {
    TestSockets sockets;
    uint8_t code = 0xAA;
    send_bytes(sockets.fd_client, &code, 1);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    EXPECT_EQ(comando, nullptr);
}

TEST(ServerProtocolCmdTest, ClosedConnectionReturnsNullptr) {
    TestSockets sockets;
    close_fd(sockets.fd_client);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    EXPECT_EQ(comando, nullptr);
}

// Tests de envio

TEST(ServerProtocolSendTest, SendLoginOk) {
    TestSockets sockets;
    {
        Socket server_socket = sockets.make_server_socket();
        ServerProtocol protocolo_server(std::move(server_socket));
        protocolo_server.send_login_ok(1234);
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_client, sizeof(uint8_t) + sizeof(uint16_t));

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::LOGIN_OK));
    uint16_t entity_id;
    memcpy(&entity_id, &buffer[sizeof(uint8_t)], sizeof(uint16_t));
    EXPECT_EQ(ntohs(entity_id), 1234);
}

TEST(ServerProtocolSendTest, SendLoginError) {
    TestSockets sockets;
    std::string mensaje_error = "Bad credentials";
    {
        Socket server_socket = sockets.make_server_socket();
        ServerProtocol protocolo_server(std::move(server_socket));
        protocolo_server.send_login_error(mensaje_error);
    }

    std::vector<uint8_t> buffer =
            recv_bytes(sockets.fd_client, sizeof(uint8_t) + sizeof(uint8_t) + mensaje_error.size());

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::LOGIN_ERROR));
    EXPECT_EQ(buffer[1], static_cast<uint8_t>(mensaje_error.size()));
    std::string mensaje_recibido(buffer.begin() + sizeof(uint8_t) + sizeof(uint8_t), buffer.end());
    EXPECT_EQ(mensaje_recibido, mensaje_error);
}

TEST(ServerProtocolSendTest, SendLoginErrorEmpty) {
    TestSockets sockets;
    {
        Socket server_socket = sockets.make_server_socket();
        ServerProtocol protocolo_server(std::move(server_socket));
        protocolo_server.send_login_error("");
    }

    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_client, sizeof(uint8_t) + sizeof(uint8_t));
    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::LOGIN_ERROR));
    EXPECT_EQ(buffer[1], 0);
}

TEST(ServerProtocolSendTest, SendMapa) {
    TestSockets sockets;
    MapaDTO mapa;
    mapa.width = 2;
    mapa.height = 2;
    TileDTO t1{10, 20, 30};
    TileDTO t2{11, 21, 31};
    TileDTO t3{12, 22, 32};
    TileDTO t4{13, 23, 33};
    mapa.tiles = {t1, t2, t3, t4};

    {
        Socket server_socket = sockets.make_server_socket();
        ServerProtocol protocolo_server(std::move(server_socket));
        protocolo_server.send_mapa(mapa);
    }

    // expected_size = type + width + height + num_tiles + 4 tiles × (3 campos uint16_t)
    size_t expected_size =
            sizeof(uint8_t) + sizeof(mapa.width) + sizeof(mapa.height) + sizeof(uint32_t) +
            mapa.tiles.size() * (sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t));
    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_client, expected_size);

    size_t pos = 0;

    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::MAPA));
    pos += sizeof(uint8_t);

    uint16_t width_recibida;
    memcpy(&width_recibida, &buffer[pos], sizeof(uint16_t));
    pos += sizeof(uint16_t);
    EXPECT_EQ(ntohs(width_recibida), mapa.width);

    uint16_t height_recibida;
    memcpy(&height_recibida, &buffer[pos], sizeof(uint16_t));
    pos += sizeof(uint16_t);
    EXPECT_EQ(ntohs(height_recibida), mapa.height);

    uint32_t num_tiles;
    memcpy(&num_tiles, &buffer[pos], sizeof(uint32_t));
    pos += sizeof(uint32_t);
    EXPECT_EQ(ntohl(num_tiles), mapa.tiles.size());

    uint16_t floor_id;
    memcpy(&floor_id, &buffer[pos], sizeof(uint16_t));
    pos += sizeof(uint16_t);
    EXPECT_EQ(ntohs(floor_id), t1.floor_id);
    uint16_t obj_id;
    memcpy(&obj_id, &buffer[pos], sizeof(uint16_t));
    pos += sizeof(uint16_t);
    EXPECT_EQ(ntohs(obj_id), t1.object_id);
    uint16_t obj_sup_id;
    memcpy(&obj_sup_id, &buffer[pos], sizeof(uint16_t));
    pos += sizeof(uint16_t);
    EXPECT_EQ(ntohs(obj_sup_id), t1.object_superior_id);
}

TEST(ServerProtocolSendTest, SendMapaEmpty) {
    TestSockets sockets;
    MapaDTO mapa;
    mapa.width = 0;
    mapa.height = 0;

    {
        Socket server_socket = sockets.make_server_socket();
        ServerProtocol protocolo_server(std::move(server_socket));
        protocolo_server.send_mapa(mapa);
    }

    // Mapa vacío: solo envía el MsgType (1 byte), después hace return
    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_client, sizeof(uint8_t));
    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::MAPA));
}

TEST(ServerProtocolSendTest, SendSnapshot) {
    TestSockets sockets;
    SnapshotDTO snap;
    snap.tick = 42;
    snap.self_entity_id = 1;
    snap.hp = 100;
    snap.max_hp = 200;
    snap.mp = 50;
    snap.max_mp = 150;
    snap.exp = 9999;
    snap.level = 5;
    snap.character_class = static_cast<uint8_t>(Class::WARRIOR);
    snap.gold = 12345;
    snap.is_ghost = 0;
    snap.meditating = 1;
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++) snap.inventory[i] = i + 1;
    snap.equipped_weapon = 1;
    snap.equipped_armor = 2;
    snap.equipped_helmet = 3;
    snap.equipped_shield = 4;

    // 1 entidad
    std::shared_ptr<std::vector<EntityDTO>> entities = std::make_shared<std::vector<EntityDTO>>();
    EntityDTO entidad;
    entidad.entity_id = 10;
    entidad.entity_type = static_cast<uint8_t>(EntityType::PLAYER);
    entidad.username = "Player1";
    entidad.pos_x = 5;
    entidad.pos_y = 6;
    entidad.direction = 1;
    entidad.sprite_id = 2;
    entidad.is_ghost = 0;
    entidad.hp_pct = 80;
    entidad.equipped_weapon = 1;
    entidad.equipped_armor = 0;
    entidad.equipped_helmet = 0;
    entidad.equipped_shield = 0;
    entities->push_back(entidad);
    snap.entities = entities;

    // 1 mensaje
    std::shared_ptr<std::vector<ChatMessageDTO>> messages =
            std::make_shared<std::vector<ChatMessageDTO>>();
    ChatMessageDTO mensaje;
    mensaje.msg_type = 0;
    mensaje.text = "Welcome!";
    messages->push_back(mensaje);
    snap.messages = messages;

    // 1 spell event
    std::shared_ptr<std::vector<SpellEventDTO>> spell_events =
            std::make_shared<std::vector<SpellEventDTO>>();
    SpellEventDTO ev;
    ev.caster_id = 10;
    ev.spell_id = 5;
    ev.target_x = 20;
    ev.target_y = 30;
    ev.is_magic_projectile = true;
    spell_events->push_back(ev);
    snap.spell_events = spell_events;

    {
        Socket server_socket = sockets.make_server_socket();
        ServerProtocol protocolo_server(std::move(server_socket));
        protocolo_server.send_snapshot(snap);
    }


    size_t size_info_snap = sizeof(uint8_t)  // MsgType::SNAPSHOT
                            + sizeof(snap.tick) + sizeof(snap.self_entity_id) + sizeof(snap.hp) +
                            sizeof(snap.max_hp) + sizeof(snap.mp) + sizeof(snap.max_mp) +
                            sizeof(snap.exp) + sizeof(snap.level) + sizeof(snap.character_class) +
                            sizeof(snap.gold) + sizeof(snap.is_ghost) + sizeof(snap.meditating);

    size_t size_inventario_snap = sizeof(uint8_t)  // INVENTORY_SIZE (cantidad de items)
                                  + SnapshotDTO::INVENTORY_SIZE * sizeof(uint8_t) +
                                  sizeof(snap.equipped_weapon) + sizeof(snap.equipped_armor) +
                                  sizeof(snap.equipped_helmet) + sizeof(snap.equipped_shield);

    size_t size_vector_entidades = sizeof(uint8_t);  // count de entidades (1)
    size_vector_entidades += sizeof(entidad.entity_id) + sizeof(entidad.entity_type) +
                             sizeof(uint8_t) + entidad.username.size()  // send_str8
                             + sizeof(entidad.pos_x) + sizeof(entidad.pos_y) +
                             sizeof(entidad.direction) + sizeof(entidad.sprite_id) +
                             sizeof(entidad.is_ghost) + sizeof(entidad.hp_pct) +
                             sizeof(entidad.equipped_weapon) + sizeof(entidad.equipped_armor) +
                             sizeof(entidad.equipped_helmet) + sizeof(entidad.equipped_shield) +
                             sizeof(entidad.level);

    size_t size_mensajes = sizeof(uint8_t);  // count de mensajes (1)
    size_mensajes += sizeof(mensaje.msg_type) + sizeof(uint8_t) + mensaje.text.size();  // send_str8

    size_t size_spell_events = sizeof(uint8_t);  // count de spell events (1)
    size_spell_events += sizeof(ev.caster_id) + sizeof(ev.spell_id) + sizeof(ev.target_x) +
                         sizeof(ev.target_y) + sizeof(ev.is_magic_projectile);

    size_t total_bytes = size_info_snap + size_inventario_snap + size_vector_entidades +
                         size_mensajes + size_spell_events;
    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_client, total_bytes);

    size_t pos = 0;  // Empezar después del MsgType
    EXPECT_EQ(buffer[pos], static_cast<uint8_t>(MsgType::SNAPSHOT));
    pos += sizeof(uint8_t);
    EXPECT_EQ(buffer.size(), total_bytes);
    SnapshotDTO snap_recibido;
    memcpy(&snap_recibido.tick, &buffer[pos], sizeof(snap_recibido.tick));
    pos += sizeof(snap_recibido.tick);
    snap_recibido.tick = ntohl(snap_recibido.tick);
    memcpy(&snap_recibido.self_entity_id, &buffer[pos], sizeof(snap_recibido.self_entity_id));
    pos += sizeof(snap_recibido.self_entity_id);
    snap_recibido.self_entity_id = ntohs(snap_recibido.self_entity_id);
    memcpy(&snap_recibido.hp, &buffer[pos], sizeof(snap_recibido.hp));
    pos += sizeof(snap_recibido.hp);
    snap_recibido.hp = ntohs(snap_recibido.hp);
    memcpy(&snap_recibido.max_hp, &buffer[pos], sizeof(snap_recibido.max_hp));
    pos += sizeof(snap_recibido.max_hp);
    snap_recibido.max_hp = ntohs(snap_recibido.max_hp);
    memcpy(&snap_recibido.mp, &buffer[pos], sizeof(snap_recibido.mp));
    pos += sizeof(snap_recibido.mp);
    snap_recibido.mp = ntohs(snap_recibido.mp);
    memcpy(&snap_recibido.max_mp, &buffer[pos], sizeof(snap_recibido.max_mp));
    pos += sizeof(snap_recibido.max_mp);
    snap_recibido.max_mp = ntohs(snap_recibido.max_mp);
    memcpy(&snap_recibido.exp, &buffer[pos], sizeof(snap_recibido.exp));
    pos += sizeof(snap_recibido.exp);
    snap_recibido.exp = ntohl(snap_recibido.exp);
    memcpy(&snap_recibido.level, &buffer[pos], sizeof(snap_recibido.level));
    pos += sizeof(snap_recibido.level);
    memcpy(&snap_recibido.character_class, &buffer[pos], sizeof(snap_recibido.character_class));
    pos += sizeof(snap_recibido.character_class);
    memcpy(&snap_recibido.gold, &buffer[pos], sizeof(snap_recibido.gold));
    pos += sizeof(snap_recibido.gold);
    snap_recibido.gold = ntohl(snap_recibido.gold);
    memcpy(&snap_recibido.is_ghost, &buffer[pos], sizeof(snap_recibido.is_ghost));
    pos += sizeof(snap_recibido.is_ghost);
    memcpy(&snap_recibido.meditating, &buffer[pos], sizeof(snap_recibido.meditating));
    pos += sizeof(snap_recibido.meditating);
    uint8_t inventory_size;
    memcpy(&inventory_size, &buffer[pos], sizeof(inventory_size));
    pos += sizeof(inventory_size);
    for (int i = 0; i < inventory_size; i++) {
        memcpy(&snap_recibido.inventory[i], &buffer[pos], sizeof(snap_recibido.inventory[i]));
        pos += sizeof(snap_recibido.inventory[i]);
    }
    memcpy(&snap_recibido.equipped_weapon, &buffer[pos], sizeof(snap_recibido.equipped_weapon));
    pos += sizeof(snap_recibido.equipped_weapon);
    memcpy(&snap_recibido.equipped_armor, &buffer[pos], sizeof(snap_recibido.equipped_armor));
    pos += sizeof(snap_recibido.equipped_armor);
    memcpy(&snap_recibido.equipped_helmet, &buffer[pos], sizeof(snap_recibido.equipped_helmet));
    pos += sizeof(snap_recibido.equipped_helmet);
    memcpy(&snap_recibido.equipped_shield, &buffer[pos], sizeof(snap_recibido.equipped_shield));
    pos += sizeof(snap_recibido.equipped_shield);
    uint8_t entity_count;
    memcpy(&entity_count, &buffer[pos], sizeof(entity_count));
    pos += sizeof(entity_count);
    EXPECT_EQ(entity_count, 1);
    EntityDTO entidad_recibida;
    memcpy(&entidad_recibida.entity_id, &buffer[pos], sizeof(entidad_recibida.entity_id));
    pos += sizeof(entidad_recibida.entity_id);
    entidad_recibida.entity_id = ntohs(entidad_recibida.entity_id);
    memcpy(&entidad_recibida.entity_type, &buffer[pos], sizeof(entidad_recibida.entity_type));
    pos += sizeof(entidad_recibida.entity_type);
    uint8_t username_len;
    memcpy(&username_len, &buffer[pos], sizeof(username_len));
    pos += sizeof(username_len);
    entidad_recibida.username =
            std::string(buffer.begin() + pos, buffer.begin() + pos + username_len);
    pos += username_len;
    memcpy(&entidad_recibida.pos_x, &buffer[pos], sizeof(entidad_recibida.pos_x));
    pos += sizeof(entidad_recibida.pos_x);
    entidad_recibida.pos_x = ntohs(entidad_recibida.pos_x);
    memcpy(&entidad_recibida.pos_y, &buffer[pos], sizeof(entidad_recibida.pos_y));
    pos += sizeof(entidad_recibida.pos_y);
    entidad_recibida.pos_y = ntohs(entidad_recibida.pos_y);
    memcpy(&entidad_recibida.direction, &buffer[pos], sizeof(entidad_recibida.direction));
    pos += sizeof(entidad_recibida.direction);
    memcpy(&entidad_recibida.sprite_id, &buffer[pos], sizeof(entidad_recibida.sprite_id));
    pos += sizeof(entidad_recibida.sprite_id);
    memcpy(&entidad_recibida.is_ghost, &buffer[pos], sizeof(entidad_recibida.is_ghost));
    pos += sizeof(entidad_recibida.is_ghost);
    memcpy(&entidad_recibida.hp_pct, &buffer[pos], sizeof(entidad_recibida.hp_pct));
    pos += sizeof(entidad_recibida.hp_pct);
    memcpy(&entidad_recibida.equipped_weapon, &buffer[pos],
           sizeof(entidad_recibida.equipped_weapon));
    pos += sizeof(entidad_recibida.equipped_weapon);
    memcpy(&entidad_recibida.equipped_armor, &buffer[pos], sizeof(entidad_recibida.equipped_armor));
    pos += sizeof(entidad_recibida.equipped_armor);
    memcpy(&entidad_recibida.equipped_helmet, &buffer[pos],
           sizeof(entidad_recibida.equipped_helmet));
    pos += sizeof(entidad_recibida.equipped_helmet);
    memcpy(&entidad_recibida.equipped_shield, &buffer[pos],
           sizeof(entidad_recibida.equipped_shield));
    pos += sizeof(entidad_recibida.equipped_shield);
    memcpy(&entidad_recibida.level, &buffer[pos], sizeof(entidad_recibida.level));
    pos += sizeof(entidad_recibida.level);
    uint8_t message_count;
    memcpy(&message_count, &buffer[pos], sizeof(message_count));
    pos += sizeof(message_count);
    EXPECT_EQ(message_count, 1);
    ChatMessageDTO mensaje_recibido;
    memcpy(&mensaje_recibido.msg_type, &buffer[pos], sizeof(mensaje_recibido.msg_type));
    pos += sizeof(mensaje_recibido.msg_type);
    uint8_t text_len;
    memcpy(&text_len, &buffer[pos], sizeof(text_len));
    pos += sizeof(text_len);
    mensaje_recibido.text = std::string(buffer.begin() + pos, buffer.begin() + pos + text_len);
    pos += text_len;
    uint8_t spell_events_count;
    memcpy(&spell_events_count, &buffer[pos], sizeof(spell_events_count));
    pos += sizeof(spell_events_count);
    EXPECT_EQ(spell_events_count, 1);
    SpellEventDTO spell_recibido;
    memcpy(&spell_recibido.caster_id, &buffer[pos], sizeof(spell_recibido.caster_id));
    pos += sizeof(spell_recibido.caster_id);
    spell_recibido.caster_id = ntohs(spell_recibido.caster_id);
    memcpy(&spell_recibido.spell_id, &buffer[pos], sizeof(spell_recibido.spell_id));
    pos += sizeof(spell_recibido.spell_id);
    memcpy(&spell_recibido.target_x, &buffer[pos], sizeof(spell_recibido.target_x));
    pos += sizeof(spell_recibido.target_x);
    spell_recibido.target_x = ntohs(spell_recibido.target_x);
    memcpy(&spell_recibido.target_y, &buffer[pos], sizeof(spell_recibido.target_y));
    pos += sizeof(spell_recibido.target_y);
    spell_recibido.target_y = ntohs(spell_recibido.target_y);
    uint8_t is_magic;
    memcpy(&is_magic, &buffer[pos], sizeof(is_magic));
    pos += sizeof(is_magic);
    EXPECT_EQ(snap_recibido.tick, snap.tick);
    EXPECT_EQ(snap_recibido.self_entity_id, snap.self_entity_id);
    EXPECT_EQ(snap_recibido.hp, snap.hp);
    EXPECT_EQ(snap_recibido.max_hp, snap.max_hp);
    EXPECT_EQ(snap_recibido.mp, snap.mp);
    EXPECT_EQ(snap_recibido.max_mp, snap.max_mp);
    EXPECT_EQ(snap_recibido.exp, snap.exp);
    EXPECT_EQ(snap_recibido.level, snap.level);
    EXPECT_EQ(snap_recibido.character_class, snap.character_class);
    EXPECT_EQ(snap_recibido.gold, snap.gold);
    EXPECT_EQ(snap_recibido.is_ghost, snap.is_ghost);
    EXPECT_EQ(snap_recibido.meditating, snap.meditating);
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++)
        EXPECT_EQ(snap_recibido.inventory[i], snap.inventory[i]);
    EXPECT_EQ(snap_recibido.equipped_weapon, snap.equipped_weapon);
    EXPECT_EQ(snap_recibido.equipped_armor, snap.equipped_armor);
    EXPECT_EQ(snap_recibido.equipped_helmet, snap.equipped_helmet);
    EXPECT_EQ(snap_recibido.equipped_shield, snap.equipped_shield);
    EXPECT_EQ(entidad_recibida.entity_id, entidad.entity_id);
    EXPECT_EQ(entidad_recibida.entity_type, entidad.entity_type);
    EXPECT_EQ(entidad_recibida.username, entidad.username);
    EXPECT_EQ(entidad_recibida.pos_x, entidad.pos_x);
    EXPECT_EQ(entidad_recibida.pos_y, entidad.pos_y);
    EXPECT_EQ(entidad_recibida.direction, entidad.direction);
    EXPECT_EQ(entidad_recibida.sprite_id, entidad.sprite_id);
    EXPECT_EQ(entidad_recibida.is_ghost, entidad.is_ghost);
    EXPECT_EQ(entidad_recibida.hp_pct, entidad.hp_pct);
    EXPECT_EQ(entidad_recibida.equipped_weapon, entidad.equipped_weapon);
    EXPECT_EQ(entidad_recibida.equipped_armor, entidad.equipped_armor);
    EXPECT_EQ(entidad_recibida.equipped_helmet, entidad.equipped_helmet);
    EXPECT_EQ(entidad_recibida.equipped_shield, entidad.equipped_shield);
    EXPECT_EQ(entidad_recibida.level, entidad.level);
    EXPECT_EQ(mensaje_recibido.msg_type, mensaje.msg_type);
    EXPECT_EQ(mensaje_recibido.text, mensaje.text);
    EXPECT_EQ(spell_recibido.caster_id, ev.caster_id);
    EXPECT_EQ(spell_recibido.spell_id, ev.spell_id);
    EXPECT_EQ(spell_recibido.target_x, ev.target_x);
    EXPECT_EQ(spell_recibido.target_y, ev.target_y);
    EXPECT_EQ(is_magic, 1);
}


TEST(ServerProtocolSendTest, SendSnapshotNoEntitiesNoMessages) {
    TestSockets sockets;
    SnapshotDTO snap;
    snap.tick = 1;
    snap.self_entity_id = 42;
    snap.hp = 100;
    snap.max_hp = 100;
    snap.mp = 50;
    snap.max_mp = 50;
    snap.exp = 0;
    snap.level = 1;
    snap.character_class = static_cast<uint8_t>(Class::MAGE);
    snap.gold = 0;
    snap.is_ghost = 0;
    snap.meditating = 0;
    for (int i = 0; i < SnapshotDTO::INVENTORY_SIZE; i++) snap.inventory[i] = 0;
    snap.equipped_weapon = 0;
    snap.equipped_armor = 0;
    snap.equipped_helmet = 0;
    snap.equipped_shield = 0;
    snap.entities = nullptr;
    snap.messages = nullptr;

    {
        Socket server_socket = sockets.make_server_socket();
        ServerProtocol protocolo_server(std::move(server_socket));
        protocolo_server.send_snapshot(snap);
    }

    size_t size_info_snap = sizeof(uint8_t)  // MsgType::SNAPSHOT
                            + sizeof(snap.tick) + sizeof(snap.self_entity_id) + sizeof(snap.hp) +
                            sizeof(snap.max_hp) + sizeof(snap.mp) + sizeof(snap.max_mp) +
                            sizeof(snap.exp) + sizeof(snap.level) + sizeof(snap.character_class) +
                            sizeof(snap.gold) + sizeof(snap.is_ghost) + sizeof(snap.meditating);

    size_t size_inventario_snap = sizeof(uint8_t)  // INVENTORY_SIZE (cantidad de items)
                                  + SnapshotDTO::INVENTORY_SIZE * sizeof(uint8_t) +
                                  sizeof(snap.equipped_weapon) + sizeof(snap.equipped_armor) +
                                  sizeof(snap.equipped_helmet) + sizeof(snap.equipped_shield);

    size_t size_entidades = sizeof(uint8_t);

    size_t size_mensajes = sizeof(uint8_t);

    size_t size_spell_events = sizeof(uint8_t);

    size_t total_bytes = size_info_snap + size_inventario_snap + size_entidades + size_mensajes +
                         size_spell_events;
    std::vector<uint8_t> buffer = recv_bytes(sockets.fd_client, total_bytes);

    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MsgType::SNAPSHOT));

    // Calcular el offset donde empieza el envio de las entidades
    size_t entities_offset = size_info_snap + size_inventario_snap;
    EXPECT_EQ(buffer[entities_offset], 0);
    size_t messages_offset = entities_offset + size_entidades;
    EXPECT_EQ(buffer[messages_offset], 0);
    size_t spell_events_offset = messages_offset + size_mensajes;
    EXPECT_EQ(buffer[spell_events_offset], 0);
}

// Test misc
TEST(ServerProtocolMiscTest, Shutdown) {
    TestSockets sockets;
    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    EXPECT_NO_THROW(protocolo_server.shutdown(SHUT_RDWR));
}

TEST(ServerProtocolMiscTest, ReceiveCommandAfterShutdownReturnsNullptr) {
    TestSockets sockets;
    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));
    protocolo_server.shutdown(SHUT_RDWR);
    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(1);
    EXPECT_EQ(comando, nullptr);
}

TEST(ServerProtocolMiscTest, FullHandshakeThenCommand) {
    TestSockets sockets;
    uint8_t login_code = static_cast<uint8_t>(MsgType::LOGIN);
    send_bytes(sockets.fd_client, &login_code, 1);

    std::string username = "TestUser";
    uint8_t uname_len = static_cast<uint8_t>(username.size());
    send_bytes(sockets.fd_client, &uname_len, 1);
    send_bytes(sockets.fd_client, username.data(), username.size());

    uint8_t move_code = static_cast<uint8_t>(MsgType::MOVE);
    uint16_t x = htons(50);
    uint16_t y = htons(60);
    send_bytes(sockets.fd_client, &move_code, 1);
    send_bytes(sockets.fd_client, &x, 2);
    send_bytes(sockets.fd_client, &y, 2);

    Socket server_socket = sockets.make_server_socket();
    ServerProtocol protocolo_server(std::move(server_socket));

    MsgType handshake = protocolo_server.receive_handshake();
    EXPECT_EQ(handshake, MsgType::LOGIN);

    std::string username_recibido;
    protocolo_server.handshake_login(username_recibido);
    EXPECT_EQ(username_recibido, username);

    std::shared_ptr<ServerCommand> comando = protocolo_server.receive_command(42);
    ASSERT_THAT(comando, NotNull());
    EXPECT_NE(dynamic_cast<MoveCommand*>(comando.get()), nullptr);
}

}  // anonymous namespace
