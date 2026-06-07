#ifndef WORLD_CHAT_H
#define WORLD_CHAT_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../../../common/protocol/dtos.h"

struct PendingMessage {
    uint16_t    to_client_id;   // 0 = broadcast
    uint8_t     msg_type;       // 0=info, 1=combat, 2=private
    std::string text;
};

class WorldChat {
private:
    std::vector<PendingMessage> pending;
public:
    void push_message(uint16_t to_id, uint8_t type, const std::string& text);
    void push_broadcast(uint8_t type, const std::string& text);

    // Mensajes para `client_id` (incluye broadcasts). Los targeteados se
    // consumen; los broadcasts persisten hasta clear_broadcasts().
    std::shared_ptr<std::vector<ChatMessageDTO>> collect(uint16_t client_id);

    void clear_broadcasts();
};

#endif
