#pragma once

// Constantes de balance/timing del server que NO vienen de un .toml en
// runtime (las que sí vienen de config/game_config.toml ya están
// centralizadas en GameConfig::formulas() — no se duplican acá).

// Cada cuántos ticks se persisten todos los jugadores conectados (ver
// ServerGameLoop::run). A diferencia del tick_rate_hz o los cooldowns,
// este intervalo no está expuesto en game_config.toml.
constexpr int GAME_AUTOSAVE_INTERVAL_TICKS = 60;
