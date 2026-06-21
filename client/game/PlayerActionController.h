#pragma once

#include <cstdint>
#include <string>

#include "../net/Command.h"
#include "../../common/queue.h"
#include "WorldState.h"
#include "PlayerState.h"

class ChatWidget;
class StatsPanel;
class GameAudioService;

// Resuelve qué pasa cuando el jugador hace click sobre una tile del mundo
// (atacar / lanzar el hechizo seleccionado / hablar con un NPC de servicio)
// y los comandos de chat dirigidos a un NPC de servicio (/listar, /curar,
// etc.): empuja Command's al servidor, agrega mensajes al chat, spawea
// efectos visuales en WorldState, y le pide los sonidos correspondientes a
// GameAudioService.
class PlayerActionController {
public:
    PlayerActionController(WorldState& state, PlayerState& player, Queue<Command>* command_queue,
                            ChatWidget* chat, StatsPanel* stats, GameAudioService& audio);

    void handle_world_click(int tile_x, int tile_y);
    void handle_chat_command(const std::string& text);

private:
    WorldState&       _state;
    PlayerState&      _player;
    Queue<Command>*   _command_queue;
    ChatWidget*       _chat;
    StatsPanel*       _stats;
    GameAudioService& _audio;
};
