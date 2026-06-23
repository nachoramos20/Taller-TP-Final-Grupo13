#include "WorldChat.h"
#include <algorithm>

void WorldChat::push_message(uint16_t to_id, uint8_t type, const std::string& text) {
    pending.push_back({to_id, type, text});
}

void WorldChat::push_broadcast(uint8_t type, const std::string& text) {
    pending.push_back({0, type, text});
}

std::shared_ptr<std::vector<ChatMessageDTO>> WorldChat::collect(uint16_t client_id) {
    auto msgs = std::make_shared<std::vector<ChatMessageDTO>>();
    for (auto it = pending.begin(); it != pending.end(); ) {
        if (it->to_client_id == 0 || it->to_client_id == client_id) {
            ChatMessageDTO m{};
            m.msg_type = it->msg_type;
            m.text     = it->text;
            msgs->push_back(m);
            if (it->to_client_id != 0) {
                it = pending.erase(it);
                continue;
            }
        }
        ++it;
    }
    return msgs;
}

void WorldChat::clear_broadcasts() {
    pending.erase(
        std::remove_if(pending.begin(), pending.end(),
            [](const PendingMessage& m){ return m.to_client_id == 0; }),
        pending.end());
}

void WorldChat::push_spell_event(uint16_t caster_id, uint8_t spell_id,
                                 uint16_t target_x, uint16_t target_y,
                                 bool is_magic_projectile) {
    pending_spell_events_.push_back({caster_id, spell_id, target_x, target_y, is_magic_projectile});
}

std::shared_ptr<std::vector<SpellEventDTO>> WorldChat::get_spell_events() const {
    return std::make_shared<std::vector<SpellEventDTO>>(pending_spell_events_);
}

void WorldChat::clear_spell_events() {
    pending_spell_events_.clear();
}
