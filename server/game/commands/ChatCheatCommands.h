#pragma once

#include <cstdint>
#include <string>

#include "../world/World.h"

// Cheats de debug invocados por chat (/set-nivel, /set-vida, etc). No
// confundir con CheatCommand (CheatCommands.cpp/.h), que son los cheats
// de atajo de teclado (Ctrl+Shift+tecla) con su propio CheatId. Extraída
// de ChatCommand para que ChatCommands.cpp no mezcle el routing de chat
// con esta lógica de debug.
class ChatCheatCommands {
public:
    explicit ChatCheatCommands(uint16_t client_id);

    void set_nivel(World& world, const std::string& args);
    void set_vida(World& world, const std::string& args);
    void set_fuerza(World& world, const std::string& args);
    void set_agilidad(World& world, const std::string& args);
    void set_inteligencia(World& world, const std::string& args);
    void set_constitucion(World& world, const std::string& args);
    void morir_instantaneo(World& world);
    void revivir_instantaneo(World& world);
    void obtener_objeto(World& world, const std::string& args);
    void set_oro(World& world, const std::string& args);

private:
    uint16_t client_id_;
};
