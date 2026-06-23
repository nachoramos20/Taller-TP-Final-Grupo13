#pragma once

#include <cstdint>
#include <string>

#include "../world/World.h"

// Operaciones de banco (depositar/retirar/listar la cuenta) para un
// cliente. Extraída de ChatCommand: antes vivía en ChatCommand_Bank.cpp
// mezclada con el catálogo del comerciante, dos responsabilidades
// distintas en el mismo archivo.
class BankCommands {
public:
    explicit BankCommands(uint16_t client_id);

    void deposit(World& world, const std::string& args);
    void withdraw(World& world, const std::string& args);

    // Si hay un banquero cerca, manda el listado de la cuenta y devuelve
    // true. Si no, devuelve false sin mandar nada (para que el caller
    // pruebe con el comerciante, ver MerchantCommands::try_list_catalog).
    bool try_list_account(World& world);

private:
    uint16_t client_id_;
};
