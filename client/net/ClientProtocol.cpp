#include "ClientProtocol.h"

#include <utility>

ClientProtocol::ClientProtocol(Socket&& socket) : Protocol(std::move(socket)) {}

void ClientProtocol::send_login(const std::string& username) {
    send_uint8(static_cast<uint8_t>(MsgType::LOGIN));
    send_string(username);
}

void ClientProtocol::send_register(const std::string& username, Race race, Class cls) {
    send_uint8(static_cast<uint8_t>(MsgType::REGISTER));
    send_string(username);
    send_uint8(static_cast<uint8_t>(race));
    send_uint8(static_cast<uint8_t>(cls));
}

void ClientProtocol::send_move(uint16_t pos_x, uint16_t pos_y) {
    send_uint8(static_cast<uint8_t>(MsgType::MOVE));
    send_uint16(pos_x);
    send_uint16(pos_y);
}

void ClientProtocol::send_attack(uint16_t target_id) {
    send_uint8(static_cast<uint8_t>(MsgType::ATTACK));
    send_uint16(target_id);
}

void ClientProtocol::send_chat_command(const std::string& command_text) {
    send_uint8(static_cast<uint8_t>(MsgType::CHAT_COMMAND));
    send_string(command_text);
}

void ClientProtocol::send_equip_item(uint8_t inv_slot) {
    send_uint8(static_cast<uint8_t>(MsgType::EQUIP_ITEM));
    send_uint8(inv_slot);
}

void ClientProtocol::send_unequip_item(EquipSlot slot) {
    send_uint8(static_cast<uint8_t>(MsgType::UNEQUIP_ITEM));
    send_uint8(static_cast<uint8_t>(slot));
}

void ClientProtocol::send_move_item(uint8_t from_slot, uint8_t to_slot) {
    send_uint8(static_cast<uint8_t>(MsgType::MOVE_ITEM));
    send_uint8(from_slot);
    send_uint8(to_slot);
}

void ClientProtocol::send_drop_item(uint8_t inv_slot) {
    send_uint8(static_cast<uint8_t>(MsgType::DROP_ITEM));
    send_uint8(inv_slot);
}

void ClientProtocol::send_pick_item() {
    send_uint8(static_cast<uint8_t>(MsgType::PICK_ITEM));
}

void ClientProtocol::send_use_item(uint8_t inv_slot) {
    send_uint8(static_cast<uint8_t>(MsgType::USE_ITEM));
    send_uint8(inv_slot);
}

void ClientProtocol::send_meditate() {
    send_uint8(static_cast<uint8_t>(MsgType::MEDITATE));
}

void ClientProtocol::send_resurrect() {
    send_uint8(static_cast<uint8_t>(MsgType::RESURRECT));
}

void ClientProtocol::send_npc_interact(uint16_t npc_id) {
    send_uint8(static_cast<uint8_t>(MsgType::NPC_INTERACT));
    send_uint16(npc_id);
}

void ClientProtocol::send_logout() {
    send_uint8(static_cast<uint8_t>(MsgType::LOGOUT));
}

void ClientProtocol::send_cast_spell(uint16_t target_id, uint8_t spell_id) {
    send_uint8(static_cast<uint8_t>(MsgType::CAST_SPELL));
    send_uint16(target_id);
    send_uint8(spell_id);
}

void ClientProtocol::send_cheat(uint8_t cheat_id) {
    send_uint8(static_cast<uint8_t>(MsgType::CHEAT));
    send_uint8(cheat_id);
}

MsgType ClientProtocol::recv_opcode() {
    return static_cast<MsgType>(recv_uint8());
}

uint16_t ClientProtocol::recv_login_ok() {
    return recv_uint16();
}

std::string ClientProtocol::recv_login_error() {
    return recv_string();
}

MapaDTO ClientProtocol::recv_map() {
    MapaDTO map;
    map.width  = recv_uint16();
    map.height = recv_uint16();
    uint32_t tile_count = recv_uint32();
    map.tiles.resize(tile_count);
    for (TileDTO& tile : map.tiles) {
        tile.floor_id           = recv_uint16();
        tile.object_id          = recv_uint16();
        tile.object_superior_id = recv_uint16();
    }
    return map;
}

SnapshotDTO ClientProtocol::recv_snapshot() {
    SnapshotDTO snapshot{};

    snapshot.tick            = recv_uint32();
    snapshot.self_entity_id  = recv_uint16();
    snapshot.hp              = recv_uint16();
    snapshot.max_hp          = recv_uint16();
    snapshot.mp              = recv_uint16();
    snapshot.max_mp          = recv_uint16();
    snapshot.exp             = recv_uint32();
    snapshot.level           = recv_uint8();
    snapshot.character_class = recv_uint8();
    snapshot.gold            = recv_uint32();
    snapshot.is_ghost        = recv_uint8();
    snapshot.meditating      = recv_uint8();

    uint8_t inv_count = recv_uint8();
    for (int i = 0; i < inv_count && i < SnapshotDTO::INVENTORY_SIZE; i++)
        snapshot.inventory[i] = recv_uint8();
    snapshot.equipped_weapon = recv_uint8();
    snapshot.equipped_armor  = recv_uint8();
    snapshot.equipped_helmet = recv_uint8();
    snapshot.equipped_shield = recv_uint8();

    uint8_t entity_count = recv_uint8();
    snapshot.entities = std::make_shared<std::vector<EntityDTO>>();
    snapshot.entities->resize(entity_count);
    for (EntityDTO& e : *snapshot.entities) {
        e.entity_id       = recv_uint16();
        e.entity_type     = recv_uint8();
        e.username        = recv_string();
        e.pos_x           = recv_uint16();
        e.pos_y           = recv_uint16();
        e.direction       = recv_uint8();
        e.sprite_id       = recv_uint8();
        e.is_ghost        = recv_uint8();
        e.hp_pct          = recv_uint8();
        e.equipped_weapon = recv_uint8();
        e.equipped_armor  = recv_uint8();
        e.equipped_helmet = recv_uint8();
        e.equipped_shield = recv_uint8();
        e.level           = recv_uint8();
    }

    uint8_t msg_count = recv_uint8();
    snapshot.messages = std::make_shared<std::vector<ChatMessageDTO>>();
    snapshot.messages->resize(msg_count);
    for (ChatMessageDTO& m : *snapshot.messages) {
        m.msg_type = recv_uint8();
        m.text     = recv_string();
    }

    uint8_t spell_event_count = recv_uint8();
    snapshot.spell_events = std::make_shared<std::vector<SpellEventDTO>>();
    snapshot.spell_events->resize(spell_event_count);
    for (SpellEventDTO& ev : *snapshot.spell_events) {
        ev.caster_id           = recv_uint16();
        ev.spell_id            = recv_uint8();
        ev.target_x            = recv_uint16();
        ev.target_y            = recv_uint16();
        ev.is_magic_projectile = recv_uint8() != 0;
    }

    return snapshot;
}
