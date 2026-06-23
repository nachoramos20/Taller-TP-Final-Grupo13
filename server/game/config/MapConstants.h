#pragma once

#include <cstdint>

// Coordenadas fijas del mapa. Nombradas con prefijo MAP_ por dominio.
// No incluye las zonas internas de generación procedural de MapaBuilder
// (ciudad/pueblo/bosque/cementerio, etc.): esas son detalle de
// implementación usado en un solo lugar, no compartidas ni duplicadas.

// Dimensiones del mapa. Las usa tanto MapaBuilder (para generarlo) como
// World/ServerGameLoop (para el tamaño del mapa de colisiones) — antes
// cada lado tenía su propio "120, 100" hardcodeado sin relación entre sí.
constexpr int MAP_WIDTH  = 120;
constexpr int MAP_HEIGHT = 100;

// Puerta del cementerio: único punto de entrada/salida de la mazmorra
// (ver DungeonCommands::enter/leave/info). Ocupa dos tiles de ancho en X
// (60 y 61) a la altura y=62.
constexpr uint16_t MAP_DUNGEON_DOOR_X1     = 60;
constexpr uint16_t MAP_DUNGEON_DOOR_X2     = 61;
constexpr uint16_t MAP_DUNGEON_DOOR_Y      = 62;
constexpr uint16_t MAP_DUNGEON_DOOR_RADIUS = 2;

// Punto de aparición al entrar a la mazmorra; al salir se vuelve a la
// puerta del cementerio (MAP_DUNGEON_DOOR_X1, MAP_DUNGEON_DOOR_Y).
constexpr uint16_t MAP_DUNGEON_SPAWN_X = 114;
constexpr uint16_t MAP_DUNGEON_SPAWN_Y = 79;
