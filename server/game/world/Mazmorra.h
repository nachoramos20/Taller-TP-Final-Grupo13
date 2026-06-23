#ifndef MAZMORRA_H
#define MAZMORRA_H

#include <cstdint>
#include <vector>
#include "../Npc.h"

class WorldNpcs;
class WorldItems;

struct MazmorraSpawnPoint {
    NpcId type;
    uint16_t x, y;
};

struct GoldSpawnPoint {
    uint16_t x, y;
    uint32_t amount;
};

// Una instancia de mazmorra: su rectángulo, los puntos de spawn de NPCs y
// oro que se repueblan cuando el último jugador la abandona, y el
// conteo de jugadores adentro (ver player_entered/player_left).
class Mazmorra {
public:
    Mazmorra(WorldNpcs& npcs, WorldItems& items,
             uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    // Registra un punto de spawn que, en cada respawn, instancia un NpcId
    // elegido al azar entre allowed_types (la elección se hace una sola vez,
    // al agregar el punto: cada respawn repite el mismo tipo en ese punto).
    void add_spawn(uint16_t x, uint16_t y, const std::vector<NpcId>& allowed_types);
    void add_gold(uint16_t x, uint16_t y, uint32_t amount);

    // Limpia la zona y vuelve a spawnear todos los puntos registrados.
    // No hace nada si hay jugadores dentro (ver player_entered).
    void respawn();

    bool activa() const { return player_count_ > 0; }
    bool in_mazmorra(uint16_t x, uint16_t y) const;
    void player_entered();
    void player_left();

private:
    WorldNpcs& npcs_;
    WorldItems& items_;
    uint16_t x1_, y1_, x2_, y2_;
    uint16_t player_count_ = 0;
    std::vector<MazmorraSpawnPoint> spawns_;
    std::vector<GoldSpawnPoint> gold_spawns_;
};

#endif