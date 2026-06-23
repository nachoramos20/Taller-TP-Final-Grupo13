#include "PlayerActionController.h"

#include <cstdlib>

#include "../../common/protocol/WeaponRules.h"
#include "../../common/protocol/protocol.h"
#include "../audio/GameAudioService.h"
#include "../ui/ChatWidget.h"
#include "../ui/StatsPanel.h"

namespace {

// Espeja Equations::is_pvp_allowed (server/game/Equations.cpp) y los valores
// de game_config.toml [formulas] (pvp_min_level=12, pvp_max_level_delta=10).
// El server sigue siendo la autoridad final: esto es solo para no spawnear
// VFX/enviar el comando de ataque cuando claramente va a ser rechazado por
// fair-play (ver Bug 1 del sistema de VFX).
bool is_pvp_allowed_client(uint8_t attacker_level, uint8_t target_level) {
    constexpr uint8_t PVP_MIN_LEVEL = 12;
    constexpr int PVP_MAX_LEVEL_DELTA = 10;
    if (attacker_level <= PVP_MIN_LEVEL || target_level <= PVP_MIN_LEVEL)
        return false;
    return std::abs(static_cast<int>(attacker_level) - static_cast<int>(target_level)) <=
           PVP_MAX_LEVEL_DELTA;
}

}  // namespace

PlayerActionController::PlayerActionController(WorldState& state, PlayerState& player,
                                               Queue<Command>* command_queue, ChatWidget* chat,
                                               StatsPanel* stats, GameAudioService& audio):
        _state(state),
        _player(player),
        _command_queue(command_queue),
        _chat(chat),
        _stats(stats),
        _audio(audio) {}

void PlayerActionController::handle_world_click(int tile_x, int tile_y) {
    for (const auto& e: _state.entities) {
        if (e.entity_id == _state.my_entity_id)
            continue;
        if (e.pos_x != tile_x || e.pos_y != tile_y)
            continue;
        if (e.entity_type == static_cast<uint8_t>(EntityType::ITEM_FLOOR))
            continue;

        float dist = _player.dist_to_player_tiles(e.pos_x, e.pos_y);
        int range_dist = _player.manhattan_dist_to_player_tiles(e.pos_x, e.pos_y);
        bool is_service_npc = (e.entity_type == static_cast<uint8_t>(EntityType::NPC)) &&
                              (e.sprite_id >= 7);  // MERCHANT=7, BANKER=8, PRIEST=9
        bool target_is_player = (e.entity_type == static_cast<uint8_t>(EntityType::PLAYER));

        // BUG FIX anim zona segura: ni el servidor permite atacar/lanzar
        // hechizos desde o hacia una zona segura (ciudad/pueblo) — no
        // spawnear el VFX/sonido cuando claramente va a ser rechazado.
        bool attack_blocked_by_safe_zone =
                !is_service_npc && (is_in_safe_zone(static_cast<uint16_t>(_player.tile_x),
                                                    static_cast<uint16_t>(_player.tile_y)) ||
                                    is_in_safe_zone(e.pos_x, e.pos_y));

        if (attack_blocked_by_safe_zone) {
            if (_chat)
                _chat->add_message("No podés atacar en una zona segura.");
            return;
        }

        if (is_service_npc) {
            if (_command_queue)
                _command_queue->push(Command::npc_interact(e.entity_id));
            if (_chat)
                _chat->add_message("Hablando con " + e.username + "...");

            auto npc = static_cast<NpcId>(e.sprite_id);
            if (npc == NpcId::MERCHANT) {
                bool already_talking = (_state.shop_npc_id == static_cast<int32_t>(e.entity_id));
                _state.shop_npc_id = e.entity_id;
                if (!already_talking)
                    _audio.merchant_greet(dist);
            } else if (npc == NpcId::BANKER) {
                bool already_talking = (_state.bank_npc_id == static_cast<int32_t>(e.entity_id));
                _state.bank_npc_id = e.entity_id;
                if (!already_talking)
                    _audio.banker_greet(dist);
            } else if (npc == NpcId::PRIEST) {
                bool already_talking = (_state.priest_npc_id == static_cast<int32_t>(e.entity_id));
                _state.priest_npc_id = e.entity_id;
                if (!already_talking)
                    _audio.priest_greet(dist);
            }
        } else if (_stats && _stats->cast_mode_active() && _stats->selected_spell() != 0) {
            // BUG FIX anim hechizo: validar maná y rango ANTES de spawnear el VFX.
            // El servidor sigue siendo la fuente de verdad — si pasa este check
            // pero el servidor rechaza (e.g. por lag), simplemente no habrá daño;
            // pero al menos no spawnear el efecto cuando claramente no es posible.

            // BUG FIX VFX fair-play: el server rechaza hechizos contra otro
            // jugador fuera del rango de fair-play, pero antes el VFX ya se
            // había spawneado en el cliente antes de que llegue el rechazo.
            if (target_is_player && !is_pvp_allowed_client(_stats->current_level(), e.level)) {
                if (_chat)
                    _chat->add_message("No puedes atacar a ese jugador (fair-play).");
                return;
            }

            uint16_t spell_mana = _stats ? _stats->selected_spell_mana_cost() : 0;
            int spell_range = _stats ? _stats->selected_spell_range() : 0;

            bool enough_mana = (_stats->current_mp() >= spell_mana);
            bool in_range = (spell_range <= 0 || range_dist <= spell_range);

            if (!enough_mana) {
                if (_chat)
                    _chat->add_message("Maná insuficiente para lanzar el hechizo.");
                return;
            }
            if (!in_range) {
                if (_chat)
                    _chat->add_message("El objetivo está fuera de rango.");
                return;
            }

            uint8_t spell = _stats->selected_spell();
            if (_command_queue)
                _command_queue->push(Command::cast_spell(e.entity_id, spell));
            _state.spawn_spell_effect(spell, e.pos_x, e.pos_y);
            _audio.spell_cast(spell, dist);
            if (_chat)
                _chat->add_message("Lanzando hechizo a " +
                                   (e.username.empty() ?
                                            std::string("#") + std::to_string(e.entity_id) :
                                            e.username));
        } else {
            // BUG FIX anim proyectil: validar rango y (para armas mágicas) maná
            // antes de spawnear el proyectil.
            uint8_t my_weapon = _state.own_weapon_item();

            // BUG FIX VFX fair-play: igual que en el hechizo, pero para
            // ataques con arma (proyectil o melee). La flauta élfica no
            // ataca (cura), así que el server la resuelve ANTES del check
            // de fair-play — hay que replicar exactamente esa excepción
            // para no bloquear curaciones legítimas a aliados fuera de rango
            // de nivel.
            bool is_flute = (my_weapon == static_cast<uint8_t>(ItemId::ELVEN_FLUTE));
            if (target_is_player && !is_flute && _stats &&
                !is_pvp_allowed_client(_stats->current_level(), e.level)) {
                if (_chat)
                    _chat->add_message("No puedes atacar a ese jugador (fair-play).");
                return;
            }

            if (weapon_is_ranged(my_weapon)) {
                int weapon_range = weapon_client_range(my_weapon);
                bool in_range = (range_dist <= weapon_range);

                // Para armas mágicas también verificar que el maná alcance
                // el costo real del arma (antes solo se chequeaba "> 0").
                bool has_mana = true;
                if (weapon_is_magic(my_weapon) && _stats) {
                    has_mana = (_stats->current_mp() >= weapon_client_mana_cost(my_weapon));
                }

                if (!in_range) {
                    if (_chat)
                        _chat->add_message("El objetivo está fuera de rango.");
                    // Igual enviamos el comando al servidor (él responde con mensaje de error),
                    // pero no spawneamos el proyectil visual.
                    if (_command_queue)
                        _command_queue->push(Command::attack(e.entity_id));
                    return;
                }
                if (!has_mana) {
                    if (_chat)
                        _chat->add_message("Sin maná para disparar el arma mágica.");
                    if (_command_queue)
                        _command_queue->push(Command::attack(e.entity_id));
                    return;
                }

                // Rango y maná OK: spawnear proyectil
                if (_command_queue)
                    _command_queue->push(Command::attack(e.entity_id));
                _state.spawn_projectile(static_cast<uint16_t>(_player.tile_x),
                                        static_cast<uint16_t>(_player.tile_y), e.pos_x, e.pos_y,
                                        weapon_is_magic(my_weapon));
                _audio.attack(my_weapon, dist);
                if (_chat) {
                    std::string who = e.username.empty() ?
                                              std::string("#") + std::to_string(e.entity_id) :
                                              e.username;
                    bool is_heal = (my_weapon == static_cast<uint8_t>(ItemId::ELVEN_FLUTE));
                    _chat->add_message((is_heal ? "Curando a " : "Atacando a ") + who);
                }
            } else {
                // Arma melee: sin proyectil, sin validación de rango client-side
                if (_command_queue)
                    _command_queue->push(Command::attack(e.entity_id));
                if (_chat)
                    _chat->add_message("Atacando a " +
                                       (e.username.empty() ?
                                                std::string("#") + std::to_string(e.entity_id) :
                                                e.username));
                _audio.attack(my_weapon, dist);
            }
        }
        return;
    }
}

void PlayerActionController::handle_chat_command(const std::string& text) {
    auto dist_to_npc = [this](int32_t npc_id) -> float {
        for (const auto& e: _state.entities)
            if (static_cast<int32_t>(e.entity_id) == npc_id)
                return _player.dist_to_player_tiles(e.pos_x, e.pos_y);
        return 0.0f;
    };

    if (_state.shop_npc_id != -1) {
        float dist = dist_to_npc(_state.shop_npc_id);
        if (text.rfind("/listar", 0) == 0)
            _audio.merchant_list_items(dist);
        else if (text.rfind("/comprar", 0) == 0)
            _audio.merchant_buy(dist);
    }

    if (_state.bank_npc_id != -1) {
        float dist = dist_to_npc(_state.bank_npc_id);
        if (text.rfind("/depositar", 0) == 0)
            _audio.banker_deposit(dist);
        else if (text.rfind("/retirar", 0) == 0)
            _audio.banker_withdraw(dist);
    }

    if (_state.priest_npc_id != -1) {
        float dist = dist_to_npc(_state.priest_npc_id);
        if (text.rfind("/curar", 0) == 0)
            _audio.priest_heal(dist);
    }
    if (text.rfind("/resucitar", 0) == 0)
        try_play_resurrect_sound();
}

void PlayerActionController::try_play_resurrect_sound() {
    for (const auto& e: _state.entities) {
        if (e.entity_type != static_cast<uint8_t>(EntityType::NPC))
            continue;
        if (static_cast<NpcId>(e.sprite_id) != NpcId::PRIEST)
            continue;
        if (_player.manhattan_dist_to_player_tiles(e.pos_x, e.pos_y) > 2)
            continue;
        _audio.priest_resurrect(_player.dist_to_player_tiles(e.pos_x, e.pos_y));
        return;
    }
}
