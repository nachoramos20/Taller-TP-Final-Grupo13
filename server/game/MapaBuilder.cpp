#include "MapaBuilder.h"

TileDTO& MapaBuilder::get_tile(MapaDTO& mapa, uint16_t x, uint16_t y) {
    return mapa.tiles[y * mapa.width + x];
}

void MapaBuilder::build_edificio(MapaDTO& mapa, uint16_t x, uint16_t y) {
    TileDTO& tile_inf_izq = mapa.tiles[y * mapa.width + x];
    tile_inf_izq.floor_id = 1;
    tile_inf_izq.object_id = 1;
    tile_inf_izq.object_superior_id = 0;


    TileDTO& tile_inf_der = mapa.tiles[y * mapa.width + (x + 1)];
    tile_inf_der.floor_id = 1;
    tile_inf_der.object_id = 1;
    tile_inf_der.object_superior_id = 0;

    TileDTO& tile_sup_izq = mapa.tiles[(y + 1) * mapa.width + x];
    tile_sup_izq.floor_id = 1;
    tile_sup_izq.object_id = 0;
    tile_sup_izq.object_superior_id = 2;

    TileDTO& tile_sup_izq2 = mapa.tiles[(y + 2) * mapa.width + x];
    tile_sup_izq2.floor_id = 1;
    tile_sup_izq2.object_id = 0;
    tile_sup_izq2.object_superior_id = 2;

    TileDTO& tile_sup_der = mapa.tiles[(y + 1) * mapa.width + (x + 1)];
    tile_sup_der.floor_id = 1;
    tile_sup_der.object_id = 0;
    tile_sup_der.object_superior_id = 2;

    TileDTO& tile_sup_der2 = mapa.tiles[(y + 2) * mapa.width + (x + 1)];
    tile_sup_der2.floor_id = 1;
    tile_sup_der2.object_id = 0;
    tile_sup_der2.object_superior_id = 2;

}

MapaDTO MapaBuilder::build_mapa_inicial() {
    MapaDTO mapa{};
    mapa.width = 100;
    mapa.height = 100;
    mapa.tiles.resize(mapa.width * mapa.height);
    for (int i = 0; i < mapa.width * mapa.height; i++) {
        mapa.tiles[i].floor_id = 1;
        mapa.tiles[i].object_id = 0;
        mapa.tiles[i].object_superior_id = 0;
    }

    build_edificio(mapa, 10, 10);
    build_edificio(mapa, 20, 20);
    build_edificio(mapa, 30, 30);

    return mapa;
}