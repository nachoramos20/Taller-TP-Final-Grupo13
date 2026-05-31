#ifndef MAPA_DTO_H
#define MAPA_DTO_H

#include <cstdint>
#include <vector>

struct TileDTO {
    uint16_t floor_id;
    uint16_t object_id;
    uint16_t object_superior_id;
};

struct MapaDTO {
    uint16_t width;
    uint16_t height;
    std::vector<TileDTO> tiles; 
};

#endif // MAPA_DTO_H