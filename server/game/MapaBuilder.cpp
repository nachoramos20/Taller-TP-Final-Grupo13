#include "MapaBuilder.h"

// floor_ids según tiles.toml
static constexpr uint16_t F_NEGRO        = 0;
static constexpr uint16_t F_PIEDRA       = 1;
static constexpr uint16_t F_PASTO_BASE   = 2;
static constexpr uint16_t F_TIERRA_BASE  = 10;
static constexpr uint16_t F_ARENA        = 81;
static constexpr uint16_t F_ARENA_FRANJA = 82;
static constexpr uint16_t F_AGUA         = 80;
static constexpr uint16_t F_AC_NORTE     = 90;
static constexpr uint16_t F_AC_SUR       = 91;
static constexpr uint16_t F_AC_OESTE     = 92;
static constexpr uint16_t F_AC_NW        = 93;
static constexpr uint16_t F_AC_NE        = 94;
static constexpr uint16_t F_AC_SW        = 95;
static constexpr uint16_t F_AC_SE        = 96;

// object_sup_ids según objects_sup.toml
static constexpr uint16_t O_MOLINO  = 1;
static constexpr uint16_t O_ARBOL   = 2;
static constexpr uint16_t O_COSTA   = 3;

// límites del mapa
static constexpr int MAP_W = 100;
static constexpr int MAP_H = 100;

// zona jugable
static constexpr int ZJ_X1 = 6,  ZJ_X2 = 93;
static constexpr int ZJ_Y1 = 6,  ZJ_Y2 = 93;

// plaza
static constexpr int PL_X1 = 33, PL_X2 = 53;
static constexpr int PL_Y1 = 35, PL_Y2 = 55;

// caminos
static constexpr int CAM_H_Y1 = 44, CAM_H_Y2 = 46;
static constexpr int CAM_V_X1 = 42, CAM_V_X2 = 44;

// costa
static constexpr int FRANJA_X1 = 75;
static constexpr int ARENA_X1  = 77, ARENA_X2  = 82;
static constexpr int OLAS_X1   = 83, OLAS_X2   = 84;
static constexpr int AGUA_X1   = 85, AGUA_X2   = 93;

//  helpers -------------------------------------------------------------

TileDTO& MapaBuilder::get_tile(MapaDTO& mapa, uint16_t x, uint16_t y) {
    return mapa.tiles[y * mapa.width + x];
}

void MapaBuilder::fill_rect(MapaDTO& mapa,
                             uint16_t x1, uint16_t y1,
                             uint16_t x2, uint16_t y2,
                             uint16_t floor_id,
                             uint16_t obj_id,
                             uint16_t obj_sup_id) {
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            TileDTO& t = get_tile(mapa, x, y);
            t.floor_id           = floor_id;
            t.object_id          = obj_id;
            t.object_superior_id = obj_sup_id;
        }
    }
}

void MapaBuilder::place_object_sup(MapaDTO& mapa, uint16_t x, uint16_t y,
                                    uint16_t obj_sup_id) {
    get_tile(mapa, x, y).object_superior_id = obj_sup_id;
}

uint16_t MapaBuilder::pasto_random() {
    return F_PASTO_BASE + (rand() % 7);
}

//  zonas --------------------------------------------------------

void MapaBuilder::build_acantilados(MapaDTO& mapa) {
    // esquinas
    get_tile(mapa, 0,  0 ).floor_id = F_AC_NW;
    get_tile(mapa, 94, 0 ).floor_id = F_AC_NE;
    get_tile(mapa, 0,  94).floor_id = F_AC_SW;
    get_tile(mapa, 94, 94).floor_id = F_AC_SE;

    // borde norte y sur — ancla cada 6 tiles
    for (int x = 6; x <= 94; x += 6) {
        get_tile(mapa, x, 0 ).floor_id = F_AC_NORTE;
        get_tile(mapa, x, 94).floor_id = F_AC_SUR;
    }

    // borde oeste — ancla cada 6 tiles
    for (int y = 6; y < 94; y += 6) {
        get_tile(mapa, 0, y).floor_id = F_AC_OESTE;
    }
}

void MapaBuilder::build_pasto(MapaDTO& mapa) {
    for (int y = ZJ_Y1; y <= ZJ_Y2; y += 2) {
        for (int x = ZJ_X1; x <= FRANJA_X1 - 1; x += 2) {
            get_tile(mapa, x, y).floor_id = pasto_random();
        }
    }
}

void MapaBuilder::build_caminos(MapaDTO& mapa) {
    for (int y = CAM_H_Y1; y <= CAM_H_Y2; y++)
        for (int x = ZJ_X1; x <= FRANJA_X1; x++)
            get_tile(mapa, x, y).floor_id = F_TIERRA_BASE + (rand() % 5);

    for (int y = ZJ_Y1; y <= ZJ_Y2; y++)
        for (int x = CAM_V_X1; x <= CAM_V_X2; x++)
            get_tile(mapa, x, y).floor_id = F_TIERRA_BASE + (rand() % 5);
}

void MapaBuilder::build_plaza(MapaDTO& mapa) {
    fill_rect(mapa, PL_X1, PL_Y1, PL_X2, PL_Y2, F_PIEDRA);
}

void MapaBuilder::build_costa(MapaDTO& mapa) {
    // franja pasto→arena — una sola columna de anclas
    for (int y = ZJ_Y1; y <= ZJ_Y2; y += 2)
        get_tile(mapa, FRANJA_X1, y).floor_id = F_ARENA_FRANJA;

    // arena pura
    for (int y = ZJ_Y1; y <= ZJ_Y2; y += 2)
        for (int x = ARENA_X1; x <= ARENA_X2; x += 2)
            get_tile(mapa, x, y).floor_id = F_ARENA;

    // arena con ola animada encima
    for (int y = ZJ_Y1; y <= ZJ_Y2; y += 2)
        for (int x = OLAS_X1; x <= OLAS_X2; x += 2) {
            get_tile(mapa, x, y).floor_id = F_ARENA;
            place_object_sup(mapa, x, y, O_COSTA);
        }

    // agua
    fill_rect(mapa, AGUA_X1, ZJ_Y1, AGUA_X2, ZJ_Y2, F_AGUA);
}

void MapaBuilder::build_objetos(MapaDTO& mapa) {
    // molinos 
    place_object_sup(mapa, PL_X1 + 3, PL_Y1 + 4, O_MOLINO);
    place_object_sup(mapa, PL_X2,     PL_Y1 + 4, O_MOLINO);
    place_object_sup(mapa, PL_X1 + 3, PL_Y2 + 1, O_MOLINO);
    place_object_sup(mapa, PL_X2,     PL_Y2 + 1, O_MOLINO);

    // árbol — centro de la plaza
    place_object_sup(mapa, (PL_X1 + PL_X2) / 2 + 1, (PL_Y1 + PL_Y2) / 2 + 3, O_ARBOL);
}

//  entry point -----------------------------------------------

MapaDTO MapaBuilder::build_mapa_inicial() {
    srand(42);

    MapaDTO mapa{};
    mapa.width  = MAP_W;
    mapa.height = MAP_H;
    mapa.tiles.resize(MAP_W * MAP_H);

    for (auto& t : mapa.tiles) {
        t.floor_id           = F_NEGRO;
        t.object_id          = 0;
        t.object_superior_id = 0;
    }

    build_pasto(mapa);
    build_caminos(mapa);
    build_plaza(mapa);
    build_costa(mapa);
    build_acantilados(mapa);
    build_objetos(mapa);

    return mapa;
}