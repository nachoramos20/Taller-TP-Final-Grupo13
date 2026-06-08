#include "MapaBuilder.h"

// floor_ids según tiles.toml
static constexpr uint16_t F_NEGRO        = 0;
static constexpr uint16_t F_PIEDRA       = 1;
static constexpr uint16_t F_PASTO_BASE   = 2;
static constexpr uint16_t F_PASTO_1X1    = 9;
static constexpr uint16_t F_TIERRA_BASE  = 10;
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
static constexpr uint16_t F_CAM_NORTE        = 68;
static constexpr uint16_t F_CAM_SUR          = 69;
static constexpr uint16_t F_CAM_ESTE         = 70;
static constexpr uint16_t F_CAM_OESTE        = 71;
static constexpr uint16_t F_CAM_INTERSECCION = 72;
static constexpr uint16_t F_FRANJA_CIU_N    = 73;
static constexpr uint16_t F_FRANJA_CIU_S    = 74;
static constexpr uint16_t F_FRANJA_CIU_E    = 75;
static constexpr uint16_t F_FRANJA_CIU_W    = 76;
static constexpr uint16_t F_FRANJA_CIU_NW   = 77;
static constexpr uint16_t F_FRANJA_CIU_NE   = 78;
static constexpr uint16_t F_FRANJA_CIU_SW   = 79;
static constexpr uint16_t F_FRANJA_CIU_SE   = 80;
static constexpr uint16_t F_FRANJA_PUE_N    = 81;
static constexpr uint16_t F_FRANJA_PUE_S    = 82;
static constexpr uint16_t F_FRANJA_PUE_E    = 83;
static constexpr uint16_t F_FRANJA_PUE_W    = 84;
static constexpr uint16_t F_FRANJA_PUE_NW   = 85;
static constexpr uint16_t F_FRANJA_PUE_NE   = 86;
static constexpr uint16_t F_FRANJA_PUE_SW   = 87;
static constexpr uint16_t F_FRANJA_PUE_SE   = 88;

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
static constexpr uint16_t O_BARRIL_FLOTANDO  = 20;
static constexpr uint16_t O_CUERNOS_FLOTANDO = 21;
static constexpr uint16_t O_ESQUELETO_1      = 22;
static constexpr uint16_t O_ESQUELETO_2      = 23;
static constexpr uint16_t O_ESQUELETO_3      = 24;
static constexpr uint16_t O_IGLESIA_CIUDAD   = 25;  
static constexpr uint16_t O_IGLESIA_PUEBLO   = 26;  
static constexpr uint16_t O_BANCO            = 27;  
static constexpr uint16_t O_COMERCIO         = 28;  
static constexpr uint16_t O_CASA_1           = 29;  
static constexpr uint16_t O_CASA_2           = 30;  
static constexpr uint16_t O_ESTATUA_IZQ      = 31;
static constexpr uint16_t O_ESTATUA_DER      = 32;
static constexpr uint16_t O_COMERCIO_CIUDAD  = 33;
static constexpr uint16_t O_CASA_CIUDAD_1    = 34;
static constexpr uint16_t O_CASA_CIUDAD_2    = 35;

// límites del mapa
static constexpr int MAP_W = 100;
static constexpr int MAP_H = 100;

// zona jugable
static constexpr int ZJ_X1 = 6,  ZJ_X2 = 93;
static constexpr int ZJ_Y1 = 6,  ZJ_Y2 = 93;

// bosque
static constexpr int BOS_X1    = 4,  BOS_X2    = 22;
static constexpr int BOS_NO_Y1 = 5,  BOS_NO_Y2 = 34;  
static constexpr int BOS_SO_Y1 = 55, BOS_SO_Y2 = 95;

// ciudad principal
static constexpr int CIU_X1 = 25, CIU_X2 = 55;
static constexpr int CIU_Y1 = 5,  CIU_Y2 = 35;

// pueblo
static constexpr int PUE_X1 = 30, PUE_X2 = 48;
static constexpr int PUE_Y1 = 54, PUE_Y2 = 70;

// cementerio
static constexpr int CEM_X1 = 55, CEM_X2 = 66;
static constexpr int CEM_Y1 = 55, CEM_Y2 = 66;

// caminos
static constexpr int CAM_H_Y1 = 39, CAM_H_Y2 = 41;
static constexpr int CAM_V_X1 = 38, CAM_V_X2 = 40;

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

    // borde norte y sur
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

    for (int y = ZJ_Y1; y <= ZJ_Y2; y++)
        for (int x = ZJ_X1; x <= FRANJA_X1 - 1; x++)
            if (get_tile(mapa, x, y).floor_id == F_NEGRO)
                get_tile(mapa, x, y).floor_id = F_PASTO_1X1;
}

void MapaBuilder::build_bosque(MapaDTO& mapa) {
    // árboles cada 4 tiles
    for (int y = BOS_NO_Y1; y <= BOS_NO_Y2; y += 4)
        for (int x = BOS_X1; x <= BOS_X2; x += 4)
            place_object_sup(mapa, x, y, arbol_bosque_random());

    for (int y = BOS_SO_Y1; y <= BOS_SO_Y2; y += 4)
        for (int x = BOS_X1; x <= BOS_X2; x += 4)
            place_object_sup(mapa, x, y, arbol_bosque_random());

    // flores dispersas entre el bosque y la ciudad
    for (int y = ZJ_Y1; y <= ZJ_Y2; y += 6)
        for (int x = BOS_X2 + 2; x <= CIU_X1 - 2; x += 6)
            if (rand() % 3 == 0)
                place_object_sup(mapa, x, y, flores_random());
}

void MapaBuilder::build_caminos(MapaDTO& mapa) {
    fill_rect(mapa, 2, CAM_H_Y1, FRANJA_X1 - 3, CAM_H_Y2, F_PASTO_1X1);

    for (int y = CIU_Y2 + 1; y <= PUE_Y1 - 1; y++)
        for (int x = CAM_V_X1; x <= CAM_V_X2; x++)
            get_tile(mapa, x, y).floor_id = F_PASTO_1X1;

    // Camino horizontal
    for (int x = 2; x <= FRANJA_X1 - 3; x += 3) {
        if (x == CAM_V_X1) continue;
        get_tile(mapa, x, CAM_H_Y1).floor_id = F_CAM_H_BASE + (rand() % F_CAM_H_COUNT);
    }
    get_tile(mapa, 2,  CAM_H_Y1).floor_id = F_CAM_OESTE;
    get_tile(mapa, 77, CAM_H_Y1).floor_id = F_CAM_ESTE;

    // Camino vertical debajo de la intersección
    for (int y = CAM_H_Y1 + 3; y < PUE_Y1 - 3; y += 3)
        get_tile(mapa, CAM_V_X1, y).floor_id = F_CAM_V_BASE + (rand() % F_CAM_V_COUNT);
    get_tile(mapa, CAM_V_X1, PUE_Y1 - 4).floor_id = F_CAM_SUR;
    get_tile(mapa, CAM_V_X1, CIU_Y2 + 2).floor_id = F_CAM_NORTE;

    // Intersección
    get_tile(mapa, CAM_V_X1, CAM_H_Y1).floor_id = F_CAM_INTERSECCION;
}

void MapaBuilder::build_ciudad(MapaDTO& mapa) {
    fill_rect(mapa, CIU_X1, CIU_Y1, CIU_X2, CIU_Y2, F_PIEDRA);

    // Franja de transición pasto->ciudad
    for (int x = CIU_X1; x <= CIU_X2; x++)
        get_tile(mapa, x, CIU_Y1 - 1).floor_id = F_FRANJA_CIU_N;
    for (int x = CIU_X1; x <= CIU_X2; x++)
        get_tile(mapa, x, CIU_Y2 + 1).floor_id = F_FRANJA_CIU_S;
    for (int y = CIU_Y1; y <= CIU_Y2; y++)
        get_tile(mapa, CIU_X2 + 1, y).floor_id = F_FRANJA_CIU_E;
    for (int y = CIU_Y1; y <= CIU_Y2; y++)
        get_tile(mapa, CIU_X1 - 1, y).floor_id = F_FRANJA_CIU_W;

    // Esquinas
    get_tile(mapa, CIU_X1 - 1, CIU_Y1 - 1).floor_id = F_FRANJA_CIU_NW;
    get_tile(mapa, CIU_X2 + 1, CIU_Y1 - 1).floor_id = F_FRANJA_CIU_NE;
    get_tile(mapa, CIU_X1 - 1, CIU_Y2 + 1).floor_id = F_FRANJA_CIU_SW;
    get_tile(mapa, CIU_X2 + 1, CIU_Y2 + 1).floor_id = F_FRANJA_CIU_SE;

    for (int x = CIU_X1 - 2; x <= CIU_X2 + 2; x++)
        get_tile(mapa, x, CIU_Y1 - 2).floor_id = F_PASTO_1X1;
    for (int y = CIU_Y1 - 2; y <= CIU_Y1; y++)
        get_tile(mapa, CIU_X1 - 2, y).floor_id = F_PASTO_1X1;

    place_object_sup(mapa, 41,  13, O_IGLESIA_CIUDAD); 
    place_object_sup(mapa, 50,  15, O_BANCO);       
    place_object_sup(mapa, 40, 20, O_FUENTE);          
    place_object_sup(mapa, 30, 17, O_COMERCIO_CIUDAD);
    place_object_sup(mapa, 29, 26, O_CASA_CIUDAD_1); 
    place_object_sup(mapa, 34, 34, O_CASA_CIUDAD_2);
    place_object_sup(mapa, 44, 34, O_CASA_CIUDAD_2);
    place_object_sup(mapa, 52, 26, O_CASA_CIUDAD_1);

    // Estatuas 
    place_object_sup(mapa, 36, 37, O_ESTATUA_IZQ);
    place_object_sup(mapa, 42, 37, O_ESTATUA_DER);
}

void MapaBuilder::build_pueblo(MapaDTO& mapa) {
    for (int y = PUE_Y1; y <= PUE_Y2; y++)
        for (int x = PUE_X1; x <= PUE_X2; x++)
            get_tile(mapa, x, y).floor_id = F_TIERRA_BASE;

    // Franja de transición pasto->pueblo
    for (int x = PUE_X1; x <= PUE_X2; x++)
        get_tile(mapa, x, PUE_Y1 - 1).floor_id = F_FRANJA_PUE_N;
    for (int x = PUE_X1; x <= PUE_X2; x++)
        get_tile(mapa, x, PUE_Y2 + 1).floor_id = F_FRANJA_PUE_S;
    for (int y = PUE_Y1; y <= PUE_Y2; y++)
        get_tile(mapa, PUE_X2 + 1, y).floor_id = F_FRANJA_PUE_E;
    for (int y = PUE_Y1; y <= PUE_Y2; y++)
        get_tile(mapa, PUE_X1 - 1, y).floor_id = F_FRANJA_PUE_W;

    // Esquinas
    get_tile(mapa, PUE_X1 - 1, PUE_Y1 - 1).floor_id = F_FRANJA_PUE_NW;
    get_tile(mapa, PUE_X2 + 1, PUE_Y1 - 1).floor_id = F_FRANJA_PUE_NE;
    get_tile(mapa, PUE_X1 - 1, PUE_Y2 + 1).floor_id = F_FRANJA_PUE_SW;
    get_tile(mapa, PUE_X2 + 1, PUE_Y2 + 1).floor_id = F_FRANJA_PUE_SE;

    for (int x = PUE_X1 - 2; x <= PUE_X2 + 2; x++)
        get_tile(mapa, x, PUE_Y1 - 2).floor_id = F_PASTO_1X1;
    for (int y = PUE_Y1 - 2; y <= PUE_Y2 + 1; y++)
        get_tile(mapa, PUE_X1 - 2, y).floor_id = F_PASTO_1X1;

    // Edificios pueblo
    place_object_sup(mapa, 40, 67, O_IGLESIA_PUEBLO);  
    place_object_sup(mapa, 46, 69, O_COMERCIO);        
    place_object_sup(mapa, 32, 69, O_CASA_1);          
    place_object_sup(mapa, 32, 62, O_CASA_2);        
}

void MapaBuilder::build_cementerio(MapaDTO& mapa) {
    for (int y = CEM_Y1; y <= CEM_Y2; y++)
        for (int x = CEM_X1; x <= CEM_X2; x++)
            get_tile(mapa, x, y).floor_id = F_TIERRA_BASE;
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

    for (int y = ZJ_Y2 + 1; y <= MAP_H; y++) {
        get_tile(mapa, FRANJA_X1, y).floor_id = F_ARENA_FRANJA;
        for (int x = ARENA_X1; x <= OLAS_X2; x++)
            get_tile(mapa, x, y).floor_id = F_ARENA;
        for (int x = AGUA_X1; x <= AGUA_X2; x++)
            get_tile(mapa, x, y).floor_id = F_AGUA;
    }
}

void MapaBuilder::build_objetos(MapaDTO& mapa) {
    // elementos en el mar
    place_object_sup(mapa, 92, 20, O_BARCO);
    place_object_sup(mapa, 93, 70, O_CANOA);
    place_object_sup(mapa, 90, 35, O_BARRIL_FLOTANDO);
    place_object_sup(mapa, 89,  5, O_BARRIL_FLOTANDO);
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
    build_costa(mapa);
    build_acantilados(mapa);
    build_caminos(mapa);
    build_ciudad(mapa);
    build_pueblo(mapa);
    build_cementerio(mapa);
    build_objetos(mapa);

    return mapa;
}
