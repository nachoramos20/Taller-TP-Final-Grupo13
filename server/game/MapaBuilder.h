#ifndef MAPA_BUILDER_H
#define MAPA_BUILDER_H

#include "../../common/MapaDTO.h"

class MapaBuilder {
    private:
        MapaDTO mapa;

        TileDTO& get_tile(MapaDTO& mapa, uint16_t x, uint16_t y);
        void build_edificio(MapaDTO& mapa, uint16_t x, uint16_t y);
    public:
        MapaBuilder() = default;
        ~MapaBuilder() = default;
        MapaDTO build_mapa_inicial();
};

#endif // MAPA_BUILDER_H