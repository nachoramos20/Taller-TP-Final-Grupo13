#ifndef MAPA_DTO_H
#define MAPA_DTO_H

#include <cstdint>
#include <vector>

// Una celda del mapa: id de piso, de objeto (decoración) y de objeto
// superior (lo que se dibuja por encima del jugador, p.ej. copas de árbol).
struct TileDTO {
    uint16_t floor_id;
    uint16_t object_id;
    uint16_t object_superior_id;
};

// Mapa completo enviado una sola vez al loguearse (ver ServerProtocol::send_mapa /
// ClientProtocol::recv_map). `tiles` está en orden row-major: y * width + x.
struct MapaDTO {
    uint16_t width;
    uint16_t height;
    std::vector<TileDTO> tiles;
};

#endif  // MAPA_DTO_H