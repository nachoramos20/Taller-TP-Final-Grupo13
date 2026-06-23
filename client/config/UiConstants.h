#pragma once

#include <SDL2/SDL.h>

// Constantes de layout/color de UI que estaban duplicadas en más de un
// lugar. No es un volcado de "todo número en la UI": la enorme mayoría
// de los SDL_Color/tamaños en InventoryRenderer, StatsPanelRenderer,
// ChatWidget y AnimationSystem son de un solo uso, ya están bien
// nombrados donde viven, y no se mueven acá sin una razón concreta
// (duplicación real) — moverlos solo relocaría código ya organizado.

// Ancho del panel de stats (client/ui/StatsPanel.h). Antes
// StatsPanelRenderer tenía su propia copia del mismo valor (250) para no
// crear un include circular con StatsPanel.h.
constexpr int UI_STATS_PANEL_WIDTH = 250;

// Proyectiles (client/render/VFXRenderer.cpp): color distinto para
// distinguir flecha de hechizo a simple vista.
constexpr SDL_Color UI_PROJECTILE_MAGIC_COLOR{120, 180, 255, 255};
constexpr SDL_Color UI_PROJECTILE_PHYSICAL_COLOR{230, 210, 120, 255};

// LoginScreen (client/ui/LoginScreen.cpp): texto de placeholder en campos
// vacíos (3 usos) y label de "Nombre de usuario:" en login/registro
// (2 usos) — antes cada aparición repetía el literal por separado.
constexpr SDL_Color UI_LOGIN_PLACEHOLDER_COLOR{80, 70, 50, 255};
constexpr SDL_Color UI_LOGIN_FIELD_LABEL_COLOR{160, 145, 100, 255};
