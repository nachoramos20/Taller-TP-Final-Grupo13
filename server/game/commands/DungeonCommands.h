#pragma once

#include <cstdint>

#include "../world/World.h"

// Entrada/salida/info de la mazmorra para un cliente. Extraída de
// ChatCommand: antes vivía en ChatCommands.cpp mezclada con comandos de
// comercio, items, social y cheats de debug.
class DungeonCommands {
public:
    explicit DungeonCommands(uint16_t client_id);

    void enter(World& world);
    void leave(World& world);
    void info(World& world);

private:
    uint16_t client_id_;
};
