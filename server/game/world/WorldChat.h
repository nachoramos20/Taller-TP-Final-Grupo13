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

// Mensajes de chat/sistema y eventos de VFX pendientes de entregar en el
// próximo snapshot de cada cliente.
class WorldChat {
private:
    std::vector<PendingMessage> pending;
    std::vector<SpellEventDTO>  pending_spell_events_;
public:
    void push_message(uint16_t to_id, uint8_t type, const std::string& text);
    void push_broadcast(uint8_t type, const std::string& text);

    // Mensajes para `client_id` (incluye broadcasts). Los targeteados se
    // consumen; los broadcasts persisten hasta clear_broadcasts().
    std::shared_ptr<std::vector<ChatMessageDTO>> collect(uint16_t client_id);

    void clear_broadcasts();

    // Eventos de VFX (ataque/hechizo exitoso) de este tick. Todo evento es
    // siempre para todos los clientes, se buscan una sola vez por tick y se 
    //comparten entre todos los snapshots de ese tick.
    void push_spell_event(uint16_t caster_id, uint8_t spell_id,
                          uint16_t target_x, uint16_t target_y,
                          bool is_magic_projectile);
    std::shared_ptr<std::vector<SpellEventDTO>> get_spell_events() const;
    void clear_spell_events();
};

#endif
