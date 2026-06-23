#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../world/World.h"
#include "../../../common/protocol/protocol.h"

// Operaciones de comerciante (comprar/vender/listar catálogo) para un
// cliente. Extraída de ChatCommand: antes "comprar"/"vender" vivían en
// ChatCommands.cpp con su propio switch de catálogo, y "listar_comerciante"
// vivía en ChatCommand_Bank.cpp con OTRO switch que tenía el mismo
// catálogo duplicado (ver Tarea 1: ambos ya se habían unificado en
// merchant_catalog_for_zone, que ahora vive acá).
class MerchantCommands {
public:
    explicit MerchantCommands(uint16_t client_id);

    void buy(World& world, const std::string& item_name);
    void sell(World& world, const std::string& item_name);

    // Si hay un comerciante cerca, manda el catálogo y devuelve true. Si
    // no, devuelve false sin mandar nada (ver BankCommands::try_list_account).
    bool try_list_catalog(World& world);

private:
    void list_catalog(World& world);

    uint16_t client_id_;
};

// Catálogo del comerciante por zona (0=Ciudad, 1=Pueblo, otro=básico).
std::vector<ItemId> merchant_catalog_for_zone(uint8_t zone_id);
