#include "MapaBuilder.h"

TileDTO& MapaBuilder::get_tile(MapaDTO& mapa, uint16_t x, uint16_t y) {
    return mapa.tiles[y * mapa.width + x];
}

void MapaBuilder::build_molino(MapaDTO& mapa, uint16_t x, uint16_t y) {
    // tile ancla: tiene el floor + el object_superior_id del molino
    TileDTO& ancla = get_tile(mapa, x, y);
    ancla.floor_id            = 1;
    ancla.object_id           = 0;
    ancla.object_superior_id  = 1;  // id 1 = molino animado
}

MapaDTO MapaBuilder::build_mapa_inicial() {
    MapaDTO mapa{};
    mapa.width  = 100;
    mapa.height = 100;
    mapa.tiles.resize(mapa.width * mapa.height);

    for (int i = 0; i < mapa.width * mapa.height; i++) {
        mapa.tiles[i].floor_id           = 1;
        mapa.tiles[i].object_id          = 0;
        mapa.tiles[i].object_superior_id = 0;
    }

    build_molino(mapa, 10, 10);
    build_molino(mapa, 20, 20);
    build_molino(mapa, 30, 30);

    return mapa;
}