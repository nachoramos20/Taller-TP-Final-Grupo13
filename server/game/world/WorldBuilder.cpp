#include "WorldBuilder.h"
#include "World.h"
#include "WorldSpawner.h"
#include "Mazmorra.h"
#include <vector>

void WorldBuilder::configure(World& world) {
    configure_spawn_zones(world);
    configure_safe_zones(world);
    configure_service_npcs(world);
    configure_dungeon(world);
}

void WorldBuilder::configure_spawn_zones(World& world) {
    WorldSpawner& spawner = world.spawner();
    spawner.set_global_cap(20);

    // Bosque NO: orcos y zombies entre los arboles del oeste.
    spawner.add_zone(SpawnZone{
        6, 6, 22, 34,
        { NpcId::ORC, NpcId::ZOMBIE },
        /*max_alive*/ 5, /*every*/ 30 * 7
    });

    // Bosque SO: misma fauna, un poco mas denso por estar lejos de la ciudad.
    spawner.add_zone(SpawnZone{
        6, 55, 22, 93,
        { NpcId::ORC, NpcId::ZOMBIE },
        /*max_alive*/ 6, /*every*/ 30 * 7
    });

    // Cementerio: no-muertos y criaturas chicas entre tumbas.
    spawner.add_zone(SpawnZone{
        55, 55, 66, 66,
        { NpcId::SKELETON, NpcId::SPIDER },
        /*max_alive*/ 5, /*every*/ 30 * 8
    });

    // Costa rocosa NE: golems como guardianes pesados lejos de los asentamientos.
    spawner.add_zone(SpawnZone{
        70, 6, 82, 34,
        { NpcId::GOLEM },
        /*max_alive*/ 3, /*every*/ 30 * 12
    });
}

void WorldBuilder::configure_safe_zones(World& world) {
    WorldSpawner& spawner = world.spawner();
    spawner.add_safe_zone(SafeZone{ 24, 4, 56, 36 });  // ciudad principal
    spawner.add_safe_zone(SafeZone{ 29, 53, 49, 71 }); // pueblo sur
}

void WorldBuilder::configure_service_npcs(World& world) {
    // ── NPCs de servicio en la ciudad ──
    world.spawn_npc_in_zone(NpcId::MERCHANT, 30, 18, 0);  // zona 0 = Ciudad
    world.spawn_npc(NpcId::BANKER,   50, 16);
    world.spawn_npc(NpcId::PRIEST,   41, 14);

    // ── NPCs de servicio en el pueblo sur ──
    world.spawn_npc(NpcId::PRIEST,   41, 68);
    world.spawn_npc_in_zone(NpcId::MERCHANT, 47, 70, 1);  // zona 1 = Pueblo
    world.spawn_npc(NpcId::BANKER,   33, 70);
}

void WorldBuilder::configure_dungeon(World& world) {
    static const std::vector<NpcId> dungeon_types = {
        NpcId::GOBLIN, NpcId::SKELETON, NpcId::ZOMBIE,
        NpcId::SPIDER, NpcId::ORC, NpcId::GOLEM
    };

    Mazmorra& mazmorra = world.add_dungeon(109, 30, 119, 80);
    for (uint16_t base_y = 34; base_y + 2 <= 75; base_y += 8) {
        for (uint16_t dx = 0; dx < 3; ++dx) {
            mazmorra.add_spawn(110 + dx, base_y, dungeon_types);
        }
    }
    for (uint16_t base_y = 34; base_y + 2 <= 75; base_y += 8) {
        for (uint16_t dx = 0; dx < 3; ++dx) {
            mazmorra.add_spawn(116 + dx, base_y, dungeon_types);
        }
    }
    mazmorra.add_gold(114, 31, 2500);
    mazmorra.add_gold(115, 31, 2500);
}
