#ifndef MAPA_BUILDER_H
#define MAPA_BUILDER_H

#include "../../common/MapaDTO.h"
#include <cstdlib>

class MapaBuilder {
private:
    TileDTO& get_tile(MapaDTO& mapa, uint16_t x, uint16_t y);

    void fill_rect(MapaDTO& mapa,
                   uint16_t x1, uint16_t y1,
                   uint16_t x2, uint16_t y2,
                   uint16_t floor_id,
                   uint16_t obj_id = 0,
                   uint16_t obj_sup_id = 0);

    void place_object_sup(MapaDTO& mapa, uint16_t x, uint16_t y,
                          uint16_t obj_sup_id);

    void build_acantilados(MapaDTO& mapa);
    void build_pasto(MapaDTO& mapa);
    void build_caminos(MapaDTO& mapa);
    void build_plaza(MapaDTO& mapa);
    void build_costa(MapaDTO& mapa);
    void build_objetos(MapaDTO& mapa);

    uint16_t pasto_random();

public:
    MapaBuilder() = default;
    ~MapaBuilder() = default;
    MapaDTO build_mapa_inicial();
};

#endif // MAPA_BUILDER_H