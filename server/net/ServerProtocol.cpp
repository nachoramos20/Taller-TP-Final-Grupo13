// Patrón aplicado: Factory + tabla de dispatch (en vez del switch de 14
// casos por MsgType). Cada entrada envuelve un receive_* existente; el
// tipo concreto que devuelve (shared_ptr<MoveCommand>, etc.) convierte
// implícitamente a shared_ptr<ServerCommand> al volver de la lambda, así
// que no hace falta tocar la firma de ningún receive_*.
#include "ServerProtocol.h"

#include <utility>
#include <stdexcept>

ServerProtocol::ServerProtocol(Socket&& socket) : Protocol(std::move(socket)) {}

const std::unordered_map<MsgType, ServerProtocol::CommandFactory>& ServerProtocol::dispatch_table() {
    static const std::unordered_map<MsgType, CommandFactory> table = {
        {MsgType::MOVE,         [](ServerProtocol& p, uint16_t id) { return p.receive_move_command(id); }},
        {MsgType::ATTACK,       [](ServerProtocol& p, uint16_t id) { return p.receive_attack(id); }},
        {MsgType::EQUIP_ITEM,   [](ServerProtocol& p, uint16_t id) { return p.receive_equip(id); }},
        {MsgType::UNEQUIP_ITEM, [](ServerProtocol& p, uint16_t id) { return p.receive_unequip(id); }},
        {MsgType::DROP_ITEM,    [](ServerProtocol& p, uint16_t id) { return p.receive_drop(id); }},
        {MsgType::MOVE_ITEM,    [](ServerProtocol& p, uint16_t id) { return p.receive_move_item(id); }},
        {MsgType::PICK_ITEM,    [](ServerProtocol& p, uint16_t id) { return p.receive_pick(id); }},
        {MsgType::USE_ITEM,     [](ServerProtocol& p, uint16_t id) { return p.receive_use(id); }},
        {MsgType::MEDITATE,     [](ServerProtocol& p, uint16_t id) { return p.receive_meditate(id); }},
        {MsgType::RESURRECT,    [](ServerProtocol& p, uint16_t id) { return p.receive_resurrect(id); }},
        {MsgType::LOGOUT,       [](ServerProtocol& p, uint16_t id) { return p.receive_logout(id); }},
        {MsgType::CHAT_COMMAND, [](ServerProtocol& p, uint16_t id) { return p.receive_chat_command(id); }},
        {MsgType::NPC_INTERACT, [](ServerProtocol& p, uint16_t id) { return p.receive_npc_interact(id); }},
        {MsgType::CAST_SPELL,   [](ServerProtocol& p, uint16_t id) { return p.receive_cast_spell(id); }},
        {MsgType::CHEAT,        [](ServerProtocol& p, uint16_t id) { return p.receive_cheat(id); }},
    };
    return table;
}

std::shared_ptr<ServerCommand> ServerProtocol::receive_command(uint16_t client_id) {
    uint8_t code;
    int n = _socket.recvall(&code, 1);
    if (n == 0) return nullptr;

    const std::unordered_map<MsgType, CommandFactory>& table = dispatch_table();
    std::unordered_map<MsgType, CommandFactory>::const_iterator it =
        table.find(static_cast<MsgType>(code));
    if (it == table.end()) return nullptr;
    return it->second(*this, client_id);
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
