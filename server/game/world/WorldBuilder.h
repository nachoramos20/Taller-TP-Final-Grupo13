#pragma once

class World;

// Arma el contenido inicial del mundo (zonas de spawn de NPCs hostiles,
// zonas seguras, NPCs de servicio y la mazmorra) antes de que arranque el
// game loop. Hoy la geografía está hardcodeada acá mismo; si se necesita
// que sea data-driven, este es el único lugar que habría que cambiar para
// leerla desde un world_config.toml en vez de literales.
class WorldBuilder {
public:
    static void configure(World& world);

private:
    static void configure_spawn_zones(World& world);
    static void configure_safe_zones(World& world);
    static void configure_service_npcs(World& world);
    static void configure_dungeon(World& world);
};
