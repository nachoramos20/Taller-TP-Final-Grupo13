#include "PlayerActionController.h"
#include "../../common/protocol/protocol.h"
#include "../audio/GameAudioService.h"
#include "ChatWidget.h"
#include "StatsPanel.h"

PlayerActionController::PlayerActionController(WorldState& state, PlayerState& player,
                                                 Queue<Command>* command_queue, ChatWidget* chat,
                                                 StatsPanel* stats, GameAudioService& audio)
    : _state(state), _player(player), _command_queue(command_queue),
      _chat(chat), _stats(stats), _audio(audio) {}

void PlayerActionController::handle_world_click(int tile_x, int tile_y) {
    for (const auto& e : _state.entities) {
        if (e.entity_id == _state.my_entity_id) continue;
        if (e.pos_x != tile_x || e.pos_y != tile_y) continue;
        // Los items en el piso (oro, sangre, drops) no son objetivos válidos:
        // si comparten tile con un NPC/jugador, no deben tapar el click sobre él.
        if (e.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR)) continue;

        float dist = dist_to_player_tiles(_player, e.pos_x, e.pos_y);
        bool is_service_npc = (e.entity_type == static_cast<uint8_t>(EntityType::NPC))
                              && (e.sprite_id >= 7);  // MERCHANT=7, BANKER=8, PRIEST=9

        if (is_service_npc) {
            if (_command_queue) _command_queue->push(Command::npc_interact(e.entity_id));
            if (_chat) _chat->add_message("Hablando con " + e.username + "...");

            auto npc = static_cast<NpcId>(e.sprite_id);
            if (npc == NpcId::MERCHANT) {
                bool already_talking = (_state.shop_npc_id == static_cast<int32_t>(e.entity_id));
                _state.shop_npc_id = e.entity_id;
                if (!already_talking) _audio.merchant_greet(dist);
            } else if (npc == NpcId::BANKER) {
                bool already_talking = (_state.bank_npc_id == static_cast<int32_t>(e.entity_id));
                _state.bank_npc_id = e.entity_id;
                if (!already_talking) _audio.banker_greet(dist);
            } else if (npc == NpcId::PRIEST) {
                bool already_talking = (_state.priest_npc_id == static_cast<int32_t>(e.entity_id));
                _state.priest_npc_id = e.entity_id;
                if (!already_talking) _audio.priest_greet(dist);
            }
        } else if (_stats && _stats->cast_mode_active() && _stats->selected_spell() != 0) {
            uint8_t spell = _stats->selected_spell();
            if (_command_queue) _command_queue->push(Command::cast_spell(e.entity_id, spell));
            spawn_spell_effect(_state, spell, e.pos_x, e.pos_y);
            _audio.spell_cast(spell, dist);
            if (_chat)
                _chat->add_message("Lanzando hechizo a " + (e.username.empty()
                    ? std::string("#") + std::to_string(e.entity_id) : e.username));
        } else {
            if (_command_queue) _command_queue->push(Command::attack(e.entity_id));
            if (_chat)
                _chat->add_message("Atacando a " + (e.username.empty()
                    ? std::string("#") + std::to_string(e.entity_id) : e.username));

            uint8_t my_weapon = own_weapon_item(_state);
            if (weapon_is_ranged(my_weapon)) {
                spawn_projectile(_state, static_cast<uint16_t>(_player.tile_x),
                                 static_cast<uint16_t>(_player.tile_y),
                                 e.pos_x, e.pos_y, weapon_is_magic(my_weapon));
            }
            _audio.attack(my_weapon, dist);
        }
        return;
    }
}

void PlayerActionController::handle_chat_command(const std::string& text) {
    auto dist_to_npc = [this](int32_t npc_id) -> float {
        for (const auto& e : _state.entities)
            if (static_cast<int32_t>(e.entity_id) == npc_id) return dist_to_player_tiles(_player, e.pos_x, e.pos_y);
        return 0.0f;
    };

    if (_state.shop_npc_id != -1) {
        float dist = dist_to_npc(_state.shop_npc_id);
        if (text.rfind("/listar", 0) == 0) _audio.merchant_list_items(dist);
        else if (text.rfind("/comprar", 0) == 0) _audio.merchant_buy(dist);
    }

    if (_state.bank_npc_id != -1) {
        float dist = dist_to_npc(_state.bank_npc_id);
        if (text.rfind("/depositar", 0) == 0) _audio.banker_deposit(dist);
        else if (text.rfind("/retirar", 0) == 0) _audio.banker_withdraw(dist);
    }

    if (_state.priest_npc_id != -1) {
        float dist = dist_to_npc(_state.priest_npc_id);
        if (text.rfind("/curar", 0) == 0) _audio.priest_heal(dist);
        else if (text.rfind("/resucitar", 0) == 0) _audio.priest_resurrect(dist);
    }
}
