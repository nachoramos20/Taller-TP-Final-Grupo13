#include "MapaBuilder.h"

// floor_ids según tiles.toml
static constexpr uint16_t F_NEGRO        = 0;
static constexpr uint16_t F_PIEDRA       = 1;
static constexpr uint16_t F_PIEDRA_SIN_MUSGO = 101;
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
static constexpr uint16_t F_CAM_INTERSECCION    = 72;
static constexpr uint16_t F_CAM_INTER_VH        = 89;
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
static constexpr uint16_t O_BANCO_PUEBLO     = 51;  
static constexpr uint16_t O_CASA_1           = 29;  
static constexpr uint16_t O_CASA_2           = 30;  
static constexpr uint16_t O_ESTATUA_IZQ      = 31;
static constexpr uint16_t O_ESTATUA_DER      = 32;
static constexpr uint16_t O_COMERCIO_CIUDAD  = 33;
static constexpr uint16_t O_CASA_CIUDAD_1    = 34;
static constexpr uint16_t O_CASA_CIUDAD_2    = 35;
// cementerio
static constexpr uint16_t O_CASA_CEM        = 36;
static constexpr uint16_t O_REJAS_CEM       = 37;
static constexpr uint16_t O_LAPIDA_2        = 38;
static constexpr uint16_t O_LAPIDA_3        = 39;
static constexpr uint16_t O_LAPIDA_4        = 40;
static constexpr uint16_t O_LAPIDA_5        = 41;
static constexpr uint16_t O_LAPIDA_6        = 42;
static constexpr uint16_t O_LINTERNA_CEM    = 43;
static constexpr uint16_t O_PALA_1          = 44;
static constexpr uint16_t O_PALA_2          = 45;
static constexpr uint16_t O_TUMBA_1         = 46;
static constexpr uint16_t O_TUMBA_2         = 47;
static constexpr uint16_t O_TUMBA_3         = 48;
static constexpr uint16_t O_TUMBA_4         = 49;
static constexpr uint16_t O_TUMBA_5         = 50;

// mazmorra

static constexpr uint16_t O_MAZMORRA_PARED_INFERIOR = 101;
static constexpr uint16_t O_MAZMORRA_PARED_IZQ = 102;
static constexpr uint16_t O_MAZMORRA_PARED_DER = 103;
static constexpr uint16_t O_MAZMORRA_PARED_SUPERIOR = 104;

// límites del mapa
static constexpr int MAP_W = 120;
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
static constexpr int CAM_V2_X1 = 59;

// costa
static constexpr int FRANJA_X1 = 82;
static constexpr int ARENA_X1  = 84, ARENA_X2  = 85;
static constexpr int OLAS_X1   = 86, OLAS_X2   = 87;
static constexpr int AGUA_X1   = 88, AGUA_X2   = 96;

MapaBuilder::MapaBuilder() {
    collision_map.resize(MAP_W * MAP_H, 0);
}

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
    mark_collision_rect(0, 0, 99, 0);   // borde norte

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
        for (int x = BOS_X1; x <= BOS_X2; x += 4) {
            mark_collision_rect(x, y, x, y);
            place_object_sup(mapa, x, y, arbol_bosque_random());
        }

    for (int y = BOS_SO_Y1; y <= BOS_SO_Y2; y += 4)
        for (int x = BOS_X1; x <= BOS_X2; x += 4)
        {
            mark_collision_rect(x, y, x, y);
            place_object_sup(mapa, x, y, arbol_bosque_random());
        }

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

    for (int y = CAM_H_Y1 + 1; y <= CEM_Y1 - 2; y++)
        for (int x = CAM_V2_X1; x <= CAM_V2_X1 + 2; x++)
            get_tile(mapa, x, y).floor_id = F_PASTO_1X1;

    // Camino horizontal
    for (int x = 2; x <= FRANJA_X1 - 3; x += 3) {
        if (x == CAM_V_X1 || x == CAM_V2_X1) continue;
        get_tile(mapa, x, CAM_H_Y1).floor_id = F_CAM_H_BASE + (rand() % F_CAM_H_COUNT);
    }
    get_tile(mapa, 2,  CAM_H_Y1).floor_id = F_CAM_OESTE;
    get_tile(mapa, 77, CAM_H_Y1).floor_id = F_CAM_ESTE;

    // Camino vertical (pueblo) debajo de la intersección
    for (int y = CAM_H_Y1 + 3; y < PUE_Y1 - 3; y += 3)
        get_tile(mapa, CAM_V_X1, y).floor_id = F_CAM_V_BASE + (rand() % F_CAM_V_COUNT);
    get_tile(mapa, CAM_V_X1, PUE_Y1 - 4).floor_id = F_CAM_SUR;
    get_tile(mapa, CAM_V_X1, CIU_Y2 + 2).floor_id = F_CAM_NORTE;

    // Camino vertical (cementerio) desde la intersección hasta la franja norte
    for (int y = CAM_H_Y1 + 3; y < CEM_Y1 - 3; y += 3)
        get_tile(mapa, CAM_V2_X1, y).floor_id = F_CAM_V_BASE + (rand() % F_CAM_V_COUNT);
    get_tile(mapa, CAM_V2_X1, CEM_Y1 - 4).floor_id = F_CAM_SUR; 

    // Intersecciones con el camino horizontal
    get_tile(mapa, CAM_V_X1,  CAM_H_Y1).floor_id = F_CAM_INTERSECCION;
    get_tile(mapa, CAM_V2_X1, CAM_H_Y1).floor_id = F_CAM_INTER_VH;
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
    mark_collision_rect(38, 7, 43, 12); // iglesia
    place_object_sup(mapa, 50,  15, O_BANCO);
    mark_collision_rect(46, 11, 54, 14); // banco
    mark_collision_rect(48,15, 52, 15); // banco entrada     
    place_object_sup(mapa, 40, 20, O_FUENTE);    
    mark_collision_rect(39, 17, 41, 17); // fuente parte superior
    mark_collision_rect(39, 20, 41, 20); // fuente parte inferior 
    mark_collision_rect(38, 18, 38, 19); // fuente parte izquierda
    mark_collision_rect(42, 18, 42, 19); // fuente parte derecha
    place_object_sup(mapa, 30, 17, O_COMERCIO_CIUDAD);
    mark_collision_rect(28, 12, 31, 17); // comercio
    mark_collision_rect(32, 14, 32, 15); // comercio casita derecja
    mark_collision_rect(27, 15, 27, 16); // comercio casita izquierda
    place_object_sup(mapa, 29, 26, O_CASA_CIUDAD_1);
    mark_collision_rect(27, 23, 30, 26); // casa ciudad 1
    place_object_sup(mapa, 34, 34, O_CASA_CIUDAD_2);
    mark_collision_rect(32, 31, 35, 34); // casa ciudad 2
    place_object_sup(mapa, 44, 34, O_CASA_CIUDAD_2);
    mark_collision_rect(42, 31, 45, 34); // casa ciudad 2
    place_object_sup(mapa, 52, 26, O_CASA_CIUDAD_1); 
    mark_collision_rect(50, 23, 53, 26); // casa ciudad 1

    // Estatuas
    place_object_sup(mapa, 36, 37, O_ESTATUA_IZQ);
    mark_collision_rect(35, 37, 37, 37); // estatua izquierda
    place_object_sup(mapa, 42, 37, O_ESTATUA_DER);
    mark_collision_rect(42, 37, 43, 37); // estatua derecha
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
    // Sur: iglesia (centro), comercio (este), banco (oeste)
    place_object_sup(mapa, 41, 67, O_IGLESIA_PUEBLO);
    mark_collision_rect(39, 62, 42, 67); // iglesia
    mark_collision_rect(43, 67, 43, 67); // iglesia pared derecha inferior
    mark_collision_rect(38, 67, 38, 67); // iglesia pared izquierda inferior
    mark_collision_rect(38, 62, 38, 62); // iglesia pared izquierda superior
    mark_collision_rect(43, 62, 43, 62); // iglesia pared derecha superior
    place_object_sup(mapa, 47, 69, O_COMERCIO);
    mark_collision_rect(45, 66, 48, 69); // comercio
    place_object_sup(mapa, 33, 69, O_BANCO_PUEBLO);
    mark_collision_rect(30, 63, 35, 69); // banco pueblo
    place_object_sup(mapa, 32, 57, O_CASA_1);
    mark_collision_rect(30, 55, 34, 57); // casa 1 
    place_object_sup(mapa, 46, 57, O_CASA_2);
    mark_collision_rect(44, 54, 48, 57); // casa 2
}

void MapaBuilder::build_cementerio(MapaDTO& mapa) {
    fill_rect(mapa, CEM_X1, CEM_Y1, CEM_X2, CEM_Y2, F_TIERRA_BASE);

    // Franja pasto <-> tierra alrededor del cementerio
    for (int x = CEM_X1; x <= CEM_X2; x++)
        get_tile(mapa, x, CEM_Y1 - 1).floor_id = F_FRANJA_PUE_N;
    for (int x = CEM_X1; x <= CEM_X2; x++)
        get_tile(mapa, x, CEM_Y2 + 1).floor_id = F_FRANJA_PUE_S;
    for (int y = CEM_Y1; y <= CEM_Y2; y++)
        get_tile(mapa, CEM_X2 + 1, y).floor_id = F_FRANJA_PUE_E;
    for (int y = CEM_Y1; y <= CEM_Y2; y++)
        get_tile(mapa, CEM_X1 - 1, y).floor_id = F_FRANJA_PUE_W;
    get_tile(mapa, CEM_X1 - 1, CEM_Y1 - 1).floor_id = F_FRANJA_PUE_NW;
    get_tile(mapa, CEM_X2 + 1, CEM_Y1 - 1).floor_id = F_FRANJA_PUE_NE;
    get_tile(mapa, CEM_X1 - 1, CEM_Y2 + 1).floor_id = F_FRANJA_PUE_SW;
    get_tile(mapa, CEM_X2 + 1, CEM_Y2 + 1).floor_id = F_FRANJA_PUE_SE;

    for (int x = CEM_X1 - 2; x <= CEM_X2 + 2; x++)
        get_tile(mapa, x, CEM_Y1 - 2).floor_id = F_PASTO_1X1;
    for (int y = CEM_Y1 - 2; y <= CEM_Y2 + 1; y++)
        get_tile(mapa, CEM_X1 - 2, y).floor_id = F_PASTO_1X1;

    mark_collision_rect(59, 66, 59, 66); // porton inf izq
    mark_collision_rect(62, 66, 62, 66); // porton inf derecho
    mark_collision_rect(59, 54, 59, 54); // porton sup izq
    mark_collision_rect(62, 54, 62, 54); // porton sup derecho
    // Rejas perimetrales completas
    place_object_sup(mapa, 61, CEM_Y2, O_REJAS_CEM);

    mark_collision_rect(55, 55, 58, 55); // reja superior horizontal izquierda
    mark_collision_rect(63, 55, 66, 55); // reja superior horizontal derecha

    mark_collision_rect(54, 55, 54, 66); // reja vertical izquierda
    mark_collision_rect(67, 55, 67, 66); // reja vertical derecha

    mark_collision_rect(55, 67, 58, 67); // reja inferior izquierda
    mark_collision_rect(63, 67, 66, 67); // reja inferior derecha




    // Casa central
    place_object_sup(mapa, 60, 61, O_CASA_CEM);
    mark_collision_rect(60,58,62,61); // casa cementerio

    // Tumbas
    place_object_sup(mapa, 57, 58, O_TUMBA_1);
    mark_collision_rect(57, 58, 57, 58); // tumba 1
    place_object_sup(mapa, 57, 61, O_TUMBA_2);
    mark_collision_rect(57, 61, 57, 61); // tumba 2
    place_object_sup(mapa, 64, 58, O_TUMBA_3);
    mark_collision_rect(64, 58, 64, 58); // tumba 3
    place_object_sup(mapa, 64, 61, O_TUMBA_4);
    mark_collision_rect(64, 61, 64, 61); // tumba 4

    // Lápidas
    place_object_sup(mapa, 56, 57, O_LAPIDA_2);
    mark_collision_rect(56, 57, 56, 57); // lapida 2
    place_object_sup(mapa, 56, 60, O_LAPIDA_3);
    mark_collision_rect(56, 60, 56, 60); // lapida 3
    place_object_sup(mapa, 65, 57, O_LAPIDA_4);
    mark_collision_rect(65, 57, 65, 57); // lapida 4
    place_object_sup(mapa, 65, 60, O_LAPIDA_5);
    mark_collision_rect(65, 60, 65, 60); // lapida 5

    // Linternas
    place_object_sup(mapa, 59, 62, O_LINTERNA_CEM);
    place_object_sup(mapa, 62, 62, O_LINTERNA_CEM);

    // Tumba, lápida y palas en el sector sur
    place_object_sup(mapa, 56, 64, O_TUMBA_5);
    mark_collision_rect(56, 64, 56, 64); // tumba 5
    place_object_sup(mapa, 63, 64, O_LAPIDA_6);
    mark_collision_rect(63, 64, 63, 64); // lapida 6
    place_object_sup(mapa, 57, 66, O_PALA_1);
    place_object_sup(mapa, 63, 66, O_PALA_2);
}

void MapaBuilder::build_costa(MapaDTO& mapa) {
    // Desde las olas (OLAS_X1) hasta el agua profunda (AGUA_X2): no se
    // puede entrar al mar. La arena (ARENA_X1..ARENA_X2) sigue caminable.
    mark_collision_rect(OLAS_X1, 1, AGUA_X2, 99);

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

    for (int y = ZJ_Y2 + 1; y < MAP_H; y++) {
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

void MapaBuilder::build_mazmorra(MapaDTO& mapa) {
    for (int y = 30; y <= 80; y++) {
        for (int x = 110; x <= 119; x++) {
            TileDTO& tile = get_tile(mapa, x, y);
            tile.floor_id = F_TIERRA_BASE;
        }
    }
    for (int y = 31; y <= 79; y++) {
        for (int x = 114; x <= 115; x++) {
            TileDTO& tile = get_tile(mapa, x, y);
            tile.floor_id = F_PIEDRA_SIN_MUSGO;
        }
    }
    place_object_sup(mapa, 112, 80, O_MAZMORRA_PARED_INFERIOR);
    place_object_sup(mapa, 117, 80, O_MAZMORRA_PARED_INFERIOR);

    place_object_sup(mapa, 112, 30, O_MAZMORRA_PARED_SUPERIOR);
    place_object_sup(mapa, 117, 30, O_MAZMORRA_PARED_SUPERIOR);


    place_object_sup(mapa, 110, 34, O_MAZMORRA_PARED_IZQ);
    for (int y = 36; y <= 80; y+= 4) {
        place_object_sup(mapa, 110, y, O_MAZMORRA_PARED_IZQ);
    }
    place_object_sup(mapa, 110, 81, O_MAZMORRA_PARED_IZQ);

    place_object_sup(mapa, 119, 34, O_MAZMORRA_PARED_DER);
    for (int y = 36; y <= 80; y+= 4) {
        place_object_sup(mapa, 119, y, O_MAZMORRA_PARED_DER);
    }
    place_object_sup(mapa, 119, 81, O_MAZMORRA_PARED_DER);

    mark_collision_rect(109, 30, 119, 30); // pared superior
    mark_collision_rect(109, 80, 119, 80); // pared inferior
    mark_collision_rect(109, 30, 109, 80); // pared izquierda la pared derecha ya queda bloqueada x limites del mapa despues hay q revisar si este es el comportamiento q esperamos o movemos la mazmorra 1 tile a la izq

}

size_t MapaBuilder::get_index(uint16_t x, uint16_t y) const {
    return static_cast<size_t>(y) * MAP_W + x;
}

void MapaBuilder::mark_collision_rect(uint16_t x1, uint16_t y1,
                                       uint16_t x2, uint16_t y2) {
    for (uint16_t y = y1; y <= y2; y++)
        for (uint16_t x = x1; x <= x2; x++)
            collision_map[get_index(x, y)] = 1;
}

std::vector<uint8_t> MapaBuilder::take_collision() {
    return std::move(collision_map);
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
    build_mazmorra(mapa);

    return mapa;
}
