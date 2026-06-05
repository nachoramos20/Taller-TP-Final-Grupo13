#include "SenderThread.h"

SenderThread::SenderThread(Socket& socket, Queue<Command>& queue)
    : _serializer(socket), _queue(queue) {}

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
            _serializer.send_login(cmd.text);
            break;
        case MsgType::REGISTER:
            _serializer.send_register(cmd.text,
                static_cast<Race>(cmd.race),
                static_cast<Class>(cmd.cls));
            break;
        case MsgType::MOVE:
            _serializer.send_move(cmd.pos_x, cmd.pos_y);
            break;
        case MsgType::ATTACK:
            _serializer.send_attack(cmd.target_id);
            break;
        case MsgType::CHAT_COMMAND:
            _serializer.send_chat_command(cmd.text);
            break;
        case MsgType::EQUIP_ITEM:
            _serializer.send_equip_item(cmd.slot);
            break;
        case MsgType::UNEQUIP_ITEM:
            _serializer.send_unequip_item(static_cast<EquipSlot>(cmd.equip_slot));
            break;
        case MsgType::DROP_ITEM:
            _serializer.send_drop_item(cmd.slot);
            break;
        case MsgType::PICK_ITEM:
            _serializer.send_pick_item();
            break;
        case MsgType::USE_ITEM:
            _serializer.send_use_item(cmd.slot);
            break;
        case MsgType::MEDITATE:
            _serializer.send_meditate();
            break;
        case MsgType::RESURRECT:
            _serializer.send_resurrect();
            break;
        case MsgType::NPC_INTERACT:
            _serializer.send_npc_interact(cmd.target_id);
            break;
        case MsgType::LOGOUT:
            _serializer.send_logout();
            break;
        default:
            break;
    }
}