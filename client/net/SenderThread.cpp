// Patrón aplicado: Factory + tabla de dispatch (en vez del switch de 17
// casos por MsgType). Cada entrada llama al send_* de ClientProtocol que
// corresponde a ese comando.
#include "SenderThread.h"

SenderThread::SenderThread(ClientProtocol& protocol, Queue<Command>& queue)
    : _protocol(protocol), _queue(queue) {}

void SenderThread::run() {
    try {
        while (true) {
            Command cmd = _queue.pop();
            send_command(cmd);
        }
    } catch (const ClosedQueue&) {}
}

void SenderThread::stop() {
    Thread::stop();
}

const std::unordered_map<MsgType, SenderThread::SendAction>& SenderThread::dispatch_table() {
    static const std::unordered_map<MsgType, SendAction> table = {
        {MsgType::LOGIN,        [](SenderThread& s, const Command& c) { s._protocol.send_login(c.text); }},
        {MsgType::REGISTER,     [](SenderThread& s, const Command& c) {
            s._protocol.send_register(c.text, static_cast<Race>(c.race), static_cast<Class>(c.cls));
        }},
        {MsgType::MOVE,         [](SenderThread& s, const Command& c) { s._protocol.send_move(c.pos_x, c.pos_y); }},
        {MsgType::ATTACK,       [](SenderThread& s, const Command& c) { s._protocol.send_attack(c.target_id); }},
        {MsgType::CHAT_COMMAND, [](SenderThread& s, const Command& c) { s._protocol.send_chat_command(c.text); }},
        {MsgType::EQUIP_ITEM,   [](SenderThread& s, const Command& c) { s._protocol.send_equip_item(c.slot); }},
        {MsgType::MOVE_ITEM,    [](SenderThread& s, const Command& c) { s._protocol.send_move_item(c.slot, c.to_slot); }},
        {MsgType::UNEQUIP_ITEM, [](SenderThread& s, const Command& c) {
            s._protocol.send_unequip_item(static_cast<EquipSlot>(c.equip_slot));
        }},
        {MsgType::DROP_ITEM,    [](SenderThread& s, const Command& c) { s._protocol.send_drop_item(c.slot); }},
        {MsgType::PICK_ITEM,    [](SenderThread& s, const Command&)   { s._protocol.send_pick_item(); }},
        {MsgType::USE_ITEM,     [](SenderThread& s, const Command& c) { s._protocol.send_use_item(c.slot); }},
        {MsgType::MEDITATE,     [](SenderThread& s, const Command&)   { s._protocol.send_meditate(); }},
        {MsgType::RESURRECT,    [](SenderThread& s, const Command&)   { s._protocol.send_resurrect(); }},
        {MsgType::NPC_INTERACT, [](SenderThread& s, const Command& c) { s._protocol.send_npc_interact(c.target_id); }},
        {MsgType::LOGOUT,       [](SenderThread& s, const Command&)   { s._protocol.send_logout(); }},
        {MsgType::CAST_SPELL,   [](SenderThread& s, const Command& c) { s._protocol.send_cast_spell(c.target_id, c.spell_id); }},
        {MsgType::CHEAT,        [](SenderThread& s, const Command& c) { s._protocol.send_cheat(c.cheat_id); }},
    };
    return table;
}

void SenderThread::send_command(const Command& cmd) {
    const auto& table = dispatch_table();
    auto it = table.find(cmd.type);
    if (it != table.end()) it->second(*this, cmd);
}
