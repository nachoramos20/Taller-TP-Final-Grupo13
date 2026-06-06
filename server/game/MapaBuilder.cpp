#include "MapaBuilder.h"

// floor_ids según tiles.toml
static constexpr uint16_t F_NEGRO        = 0;
static constexpr uint16_t F_PIEDRA       = 1;
static constexpr uint16_t F_PASTO_BASE   = 2;   
static constexpr uint16_t F_TIERRA_BASE  = 10;  
static constexpr uint16_t F_TIERRA_COUNT = 4;
static constexpr uint16_t F_FRANJA_IZQ   = 40;
static constexpr uint16_t F_FRANJA_DER   = 41;
static constexpr uint16_t F_FRANJA_SUP   = 42;
static constexpr uint16_t F_FRANJA_INF   = 43;
static constexpr uint16_t F_AGUA         = 44;
static constexpr uint16_t F_ARENA        = 45;
static constexpr uint16_t F_ARENA_FRANJA = 46;
static constexpr uint16_t F_AC_NORTE     = 50;
static constexpr uint16_t F_AC_SUR       = 51;
static constexpr uint16_t F_AC_OESTE     = 52;
static constexpr uint16_t F_AC_NW        = 53;
static constexpr uint16_t F_AC_NE        = 54;
static constexpr uint16_t F_AC_SW        = 55;
static constexpr uint16_t F_AC_SE        = 56;
// caminos verticales
static constexpr uint16_t F_CAM_V_BASE   = 60;
static constexpr uint16_t F_CAM_V_COUNT  = 4;
// caminos horizontales 
static constexpr uint16_t F_CAM_H_BASE   = 64;
static constexpr uint16_t F_CAM_H_COUNT  = 4;
// terminaciones de camino 
static constexpr uint16_t F_CAM_NORTE       = 68;
static constexpr uint16_t F_CAM_SUR         = 69;
static constexpr uint16_t F_CAM_ESTE        = 70;
static constexpr uint16_t F_CAM_OESTE       = 71;
static constexpr uint16_t F_CAM_INTERSECCION = 72;

// object_sup_ids
static constexpr uint16_t O_MOLINO           = 1;
static constexpr uint16_t O_FUENTE           = 2;
static constexpr uint16_t O_COSTA            = 3;
static constexpr uint16_t O_ARBOL_RECTO_1    = 4;
static constexpr uint16_t O_ARBOL_RECTO_2    = 5;
static constexpr uint16_t O_ARBOL_RECTO_3    = 6;
static constexpr uint16_t O_PINO_RECTO       = 7;
static constexpr uint16_t O_PINO_RECTO_2     = 8;
static constexpr uint16_t O_PINO_TORCIDO     = 9;
static constexpr uint16_t O_PINO_TORCIDO_IZQ = 10;
static constexpr uint16_t O_BARCO            = 11;
static constexpr uint16_t O_CANOA            = 12;
static constexpr uint16_t O_FLORES_BASE      = 13;
static constexpr uint16_t O_BARRIL_FLOTANDO    = 20;
static constexpr uint16_t O_CUERNOS_FLOTANDO   = 21;
static constexpr uint16_t O_ESQUELETO_1        = 22;
static constexpr uint16_t O_ESQUELETO_2        = 23;
static constexpr uint16_t O_ESQUELETO_3        = 24;

// límites del mapa
static constexpr int MAP_W = 100;
static constexpr int MAP_H = 100;

// zona jugable
static constexpr int ZJ_X1 = 6,  ZJ_X2 = 93;
static constexpr int ZJ_Y1 = 6,  ZJ_Y2 = 93;

// bosque
static constexpr int BOS_X1    = 6,  BOS_X2    = 18;
static constexpr int BOS_NO_Y1 = 6,  BOS_NO_Y2 = 45;
static constexpr int BOS_SO_Y1 = 55, BOS_SO_Y2 = 93;

// ciudad principal
static constexpr int CIU_X1 = 25, CIU_X2 = 75;
static constexpr int CIU_Y1 = 5,  CIU_Y2 = 45;

// pueblo — debajo del camino horizontal
static constexpr int PUE_X1 = 25, PUE_X2 = 75;
static constexpr int PUE_Y1 = 55, PUE_Y2 = 95;

// caminos — horizontal pasa justo debajo de la ciudad 
static constexpr int CAM_H_Y1 = 47, CAM_H_Y2 = 49;
// vertical centrado en x de la ciudad 
static constexpr int CAM_V_X1 = 49, CAM_V_X2 = 51;

// costa
static constexpr int FRANJA_X1 = 82;
static constexpr int ARENA_X1  = 84, ARENA_X2  = 85;
static constexpr int OLAS_X1   = 86, OLAS_X2   = 87;
static constexpr int AGUA_X1   = 88, AGUA_X2   = 99;

// HELPERS

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

uint16_t MapaBuilder::arbol_bosque_random() {
    static const uint16_t arboles[] = {
        O_ARBOL_RECTO_1, O_ARBOL_RECTO_2, O_ARBOL_RECTO_3,
        O_PINO_RECTO, O_PINO_RECTO_2,
        O_PINO_TORCIDO, O_PINO_TORCIDO_IZQ
    };
    return arboles[rand() % 7];
}

uint16_t MapaBuilder::flores_random() {
    return O_FLORES_BASE + (rand() % 7);
}

// ZONAS 

void MapaBuilder::build_acantilados(MapaDTO& mapa) {
    // esquinas oeste
    get_tile(mapa, 0,  1 ).floor_id = F_AC_NW;
    get_tile(mapa, 0,  98).floor_id = F_AC_SW;

    // borde norte y sur — de 6 a 70 + esquina en 76
    for (int x = 0; x <= 76; x += 6) {
        get_tile(mapa, x, 0 ).floor_id = F_AC_NORTE;
        get_tile(mapa, x, 99).floor_id = F_AC_SUR;
    }

    // esquinas este
    get_tile(mapa, 76, 0 ).floor_id = F_AC_NE;
    get_tile(mapa, 76, 99).floor_id = F_AC_SE;

    // borde oeste
    for (int y = 6; y < 99; y += 6)
        get_tile(mapa, 0, y).floor_id = F_AC_OESTE;
}

void MapaBuilder::build_pasto(MapaDTO& mapa) {
    // Pasto principal
    for (int y = ZJ_Y1; y <= 96; y += 2)
        for (int x = ZJ_X1; x <= FRANJA_X1 - 1; x += 2)
            get_tile(mapa, x, y).floor_id = pasto_random();

    for (int y = ZJ_Y1; y <= 98; y += 2)
        get_tile(mapa, 53, y).floor_id = pasto_random();

    for (int y = 1; y <= ZJ_Y1 - 1; y++)
        for (int x = 0; x <= FRANJA_X1 - 1; x++)
            get_tile(mapa, x, y).floor_id = F_PASTO_BASE;

    for (int y = ZJ_Y2 + 1; y <= 98; y++)
        for (int x = 0; x <= FRANJA_X1 - 1; x++)
            get_tile(mapa, x, y).floor_id = F_PASTO_BASE;

    for (int y = 1; y <= ZJ_Y2; y++)
        for (int x = 0; x <= ZJ_X1 - 1; x++)
            get_tile(mapa, x, y).floor_id = F_PASTO_BASE;
}

void MapaBuilder::build_bosque(MapaDTO& mapa) {
    // árboles cada 4 tiles
    for (int y = BOS_NO_Y1; y <= BOS_NO_Y2; y += 4)
        for (int x = BOS_X1; x <= BOS_X2; x += 4)
            place_object_sup(mapa, x, y, arbol_bosque_random());

    for (int y = BOS_SO_Y1; y <= BOS_SO_Y2; y += 4)
        for (int x = BOS_X1; x <= BOS_X2; x += 4)
            place_object_sup(mapa, x, y, arbol_bosque_random());

    // flores dispersas en el campo
    for (int y = ZJ_Y1; y <= ZJ_Y2; y += 6)
        for (int x = BOS_X2 + 2; x <= CIU_X1 - 2; x += 6)
            if (rand() % 3 == 0)
                place_object_sup(mapa, x, y, flores_random());
}

void MapaBuilder::build_caminos(MapaDTO& mapa) {
    // camino horizontal
    for (int x = ZJ_X1; x <= FRANJA_X1 - 2; x += 2) {
        uint16_t var = F_CAM_H_BASE + (rand() % F_CAM_H_COUNT);
        get_tile(mapa, x, CAM_H_Y1 + 1).floor_id = var;
    }
    // terminaciones norte y sur del camino horizontal
    for (int x = ZJ_X1; x <= FRANJA_X1 - 1; x++) {
        get_tile(mapa, x, CAM_H_Y1).floor_id = F_CAM_NORTE;
        get_tile(mapa, x, CAM_H_Y2).floor_id = F_CAM_SUR;
    }

    // camino vertical
    for (int y = ZJ_Y1; y <= ZJ_Y2 - 1; y += 2) {
        uint16_t var = F_CAM_V_BASE + (rand() % F_CAM_V_COUNT);
        get_tile(mapa, CAM_V_X1 + 1, y).floor_id = var;
    }
    // terminaciones oeste y este del camino vertical
    for (int y = ZJ_Y1; y <= ZJ_Y2; y++) {
        get_tile(mapa, CAM_V_X1, y).floor_id = F_CAM_OESTE;
        get_tile(mapa, CAM_V_X2, y).floor_id = F_CAM_ESTE;
    }
    //  intersección
    get_tile(mapa, CAM_V_X1 + 1, CAM_H_Y1 + 1).floor_id = F_CAM_INTERSECCION;
}

void MapaBuilder::build_ciudad(MapaDTO& mapa) {
    fill_rect(mapa, CIU_X1, CIU_Y1, CIU_X2, CIU_Y2, F_PIEDRA);
}

void MapaBuilder::build_pueblo(MapaDTO& mapa) {
    for (int y = PUE_Y1; y <= PUE_Y2; y++)
        for (int x = PUE_X1; x <= PUE_X2; x++)
            get_tile(mapa, x, y).floor_id = F_TIERRA_BASE + (rand() % F_TIERRA_COUNT);
}

void MapaBuilder::build_costa(MapaDTO& mapa) {
    // franja pasto->arena
    for (int y = 1; y <= 99; y += 2)
        get_tile(mapa, FRANJA_X1, y).floor_id = F_ARENA_FRANJA;

    // arena pura
    for (int y = 1; y <= 99; y++)
        for (int x = ARENA_X1; x <= OLAS_X2; x++)
            get_tile(mapa, x, y).floor_id = F_ARENA;

    // olas animadas
    for (int y = 1; y <= 99; y++)
        for (int x = OLAS_X1; x <= OLAS_X2; x += 2)
            place_object_sup(mapa, x, y, O_COSTA);

    // agua
    for (int y = 1; y <= 99; y++)
        for (int x = AGUA_X1; x <= AGUA_X2; x++)
            get_tile(mapa, x, y).floor_id = F_AGUA;

    // costa en borde norte
    // costa en borde sur 
    for (int y = ZJ_Y2 + 1; y <= MAP_H; y++) {
        get_tile(mapa, FRANJA_X1, y).floor_id = F_ARENA_FRANJA;
        for (int x = ARENA_X1; x <= OLAS_X2; x++)
            get_tile(mapa, x, y).floor_id = F_ARENA;
        for (int x = AGUA_X1; x <= AGUA_X2; x++)
            get_tile(mapa, x, y).floor_id = F_AGUA;
    }

}

void MapaBuilder::build_objetos(MapaDTO& mapa) {
    // molinos en las 4 esquinas interiores de la ciudad
    place_object_sup(mapa, CIU_X1 + 3, CIU_Y1 + 4, O_MOLINO);
    place_object_sup(mapa, CIU_X2 - 3, CIU_Y1 + 4, O_MOLINO);
    place_object_sup(mapa, CIU_X1 + 3, CIU_Y2 - 3, O_MOLINO);
    place_object_sup(mapa, CIU_X2 - 3, CIU_Y2 - 3, O_MOLINO);

    // fuente en el centro de la ciudad
    place_object_sup(mapa, (CIU_X1 + CIU_X2) / 2,
                           (CIU_Y1 + CIU_Y2) / 2, O_FUENTE);

    // elementos en el mar
    place_object_sup(mapa, 92, 20, O_BARCO);
    place_object_sup(mapa, 93, 70, O_CANOA);
    place_object_sup(mapa, 90, 35, O_BARRIL_FLOTANDO);
    place_object_sup(mapa, 89, 5, O_BARRIL_FLOTANDO);
    place_object_sup(mapa, 95, 15, O_CUERNOS_FLOTANDO);
    place_object_sup(mapa, 95, 30, O_CUERNOS_FLOTANDO);
    place_object_sup(mapa, 91, 55, O_ESQUELETO_1);
    place_object_sup(mapa, 94, 80, O_ESQUELETO_2);
    place_object_sup(mapa, 97, 45, O_ESQUELETO_3);
    place_object_sup(mapa, 96, 75, O_ESQUELETO_1);
}

// ENTRY POINT

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
    build_bosque(mapa);
    build_caminos(mapa);
    build_ciudad(mapa);
    build_pueblo(mapa);
    build_costa(mapa);
    build_acantilados(mapa); 
    build_objetos(mapa);

    return mapa;
}