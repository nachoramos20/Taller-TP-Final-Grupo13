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

void SenderThread::send_command(const Command& cmd) {
    switch (cmd.type) {
        case MsgType::LOGIN:
            _protocol.send_login(cmd.text);
            break;
        case MsgType::REGISTER:
            _protocol.send_register(cmd.text,
                static_cast<Race>(cmd.race),
                static_cast<Class>(cmd.cls));
            break;
        case MsgType::MOVE:
            _protocol.send_move(cmd.pos_x, cmd.pos_y);
            break;
        case MsgType::ATTACK:
            _protocol.send_attack(cmd.target_id);
            break;
        case MsgType::CHAT_COMMAND:
            _protocol.send_chat_command(cmd.text);
            break;
        case MsgType::EQUIP_ITEM:
            _protocol.send_equip_item(cmd.slot);
            break;
        case MsgType::MOVE_ITEM:
            _protocol.send_move_item(cmd.slot, cmd.to_slot);
            break;
        case MsgType::UNEQUIP_ITEM:
            _protocol.send_unequip_item(static_cast<EquipSlot>(cmd.equip_slot));
            break;
        case MsgType::DROP_ITEM:
            _protocol.send_drop_item(cmd.slot);
            break;
        case MsgType::PICK_ITEM:
            _protocol.send_pick_item();
            break;
        case MsgType::USE_ITEM:
            _protocol.send_use_item(cmd.slot);
            break;
        case MsgType::MEDITATE:
            _protocol.send_meditate();
            break;
        case MsgType::RESURRECT:
            _protocol.send_resurrect();
            break;
        case MsgType::NPC_INTERACT:
            _protocol.send_npc_interact(cmd.target_id);
            break;
        case MsgType::LOGOUT:
            _protocol.send_logout();
            break;
        case MsgType::CAST_SPELL:
            _protocol.send_cast_spell(cmd.target_id, cmd.spell_id);
            break;
        case MsgType::CHEAT:
            _protocol.send_cheat(cmd.cheat_id);
            break;
        default:
            break;
    }
}
