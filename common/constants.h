#pragma once

// Constantes compartidas entre client y server (tamaños/límites del
// protocolo de red). No incluye nada que ya venga de un .toml en runtime
// (eso vive en GameConfig/AudioConfig/etc., no acá).

// Cantidad de slots de inventario que viajan por el protocolo
// (SnapshotDTO) y que el server guarda por jugador (PlayerData). Antes
// estaba definida por separado en dtos.h y en PlayerData.h con el mismo
// valor (20) sin relación entre sí.
constexpr int PROTOCOL_INVENTORY_SIZE = 20;
