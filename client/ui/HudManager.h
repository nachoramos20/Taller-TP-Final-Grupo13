#pragma once

#include <memory>

#include "ChatWidget.h"
#include "StatsPanel.h"
#include "InventoryPanel.h"
#include "PositionLabel.h"

class GameAudioService;

// Agrupa los cuatro widgets de HUD del GameLoop (chat, stats, inventario,
// indicador de posición), que antes GameLoop instanciaba y renderizaba uno
// por uno. Construye los cuatro con las fuentes de ClientConfig; siempre
// están presentes (no hay modo "sin HUD"), así que los accesores devuelven
// punteros crudos no nulos para no romper la interfaz que ya esperan
// SnapshotProcessor/InputController/PlayerActionController.
class HudManager {
public:
    HudManager(SDL2pp::Renderer& renderer, GameAudioService* audio);

    ChatWidget*     chat()           { return _chat.get(); }
    StatsPanel*     stats()          { return _stats.get(); }
    InventoryPanel* inventory()      { return _inventory.get(); }
    PositionLabel*  position_label() { return _pos_label.get(); }

    void render(int screen_w, int screen_h);

private:
    std::unique_ptr<ChatWidget>     _chat;
    std::unique_ptr<StatsPanel>     _stats;
    std::unique_ptr<InventoryPanel> _inventory;
    std::unique_ptr<PositionLabel>  _pos_label;
};
