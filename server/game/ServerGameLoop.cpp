#include "ServerGameLoop.h"
#include "PlayerData.h"
#include "Stats.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

ServerGameLoop::ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                               QueueMonitor& queue_monitor,
                               Queue<PlayerData>& save_queue,
                               std::vector<uint8_t> collision_map)
    : command_queue(command_queue),
      queue_monitor(queue_monitor),
      save_queue(save_queue),
       world(120, 100, std::move(collision_map), save_queue), tick(0),
      regen_ticks(0) {}

void ServerGameLoop::run() {
    using clock = std::chrono::steady_clock;
    auto next_tick = clock::now();

    // ── Configuración del spawner ──
    auto& sp = world.spawner();
    sp.set_global_cap(20);

    // Bosque NO: orcos y zombies entre los arboles del oeste.
    sp.add_zone(SpawnZone{
        6, 6, 22, 34,
        { NpcId::ORC, NpcId::ZOMBIE },
        /*max_alive*/ 5, /*every*/ 30 * 7
    });

    // Bosque SO: misma fauna, un poco mas denso por estar lejos de la ciudad.
    sp.add_zone(SpawnZone{
        6, 55, 22, 93,
        { NpcId::ORC, NpcId::ZOMBIE },
        /*max_alive*/ 6, /*every*/ 30 * 7
    });

    // Cementerio: no-muertos y criaturas chicas entre tumbas.
    sp.add_zone(SpawnZone{
        55, 55, 66, 66,
        { NpcId::SKELETON, NpcId::SPIDER },
        /*max_alive*/ 5, /*every*/ 30 * 8
    });

    // Costa rocosa NE: golems como guardianes pesados lejos de los asentamientos.
    sp.add_zone(SpawnZone{
        70, 6, 82, 34,
        { NpcId::GOLEM },
        /*max_alive*/ 3, /*every*/ 30 * 12
    });

    // ── Zonas seguras (asentamientos) ──
    sp.add_safe_zone(SafeZone{ 24, 4, 56, 36 });  // ciudad principal
    sp.add_safe_zone(SafeZone{ 29, 53, 49, 71 }); // pueblo sur

    // ── NPCs de servicio en la ciudad ──
    world.spawn_npc_in_zone(NpcId::MERCHANT, 30, 18, 0);  // zona 0 = Ciudad
    world.spawn_npc(NpcId::BANKER,   50, 16);
    world.spawn_npc(NpcId::PRIEST,   41, 14);

    // ── NPCs de servicio en el pueblo sur ──
    world.spawn_npc(NpcId::PRIEST,   41, 68);
    world.spawn_npc_in_zone(NpcId::MERCHANT, 47, 70, 1);  // zona 1 = Pueblo
    world.spawn_npc(NpcId::BANKER,   33, 70);

    // mazmorra
    Mazmorra& mazmorra = world.add_dungeon(92,4, 99,99);
    //inicio 95,98
    mazmorra.add_spawn(NpcId::ORC, 95, 95);

    while (should_keep_running()) {
        process_commands();
        update();
        broadcast_snapshots();
        world.clear_broadcast_messages();

        tick++;
        if (tick % 60 == 0) save_players();
        next_tick += std::chrono::milliseconds(SERVER_TICK_MS);
        std::this_thread::sleep_until(next_tick);
    }
}


void ServerGameLoop::stop() {
    save_players();
    Thread::stop();
}

void ServerGameLoop::process_commands() {
    std::shared_ptr<ServerCommand> command;
    while (command_queue.try_pop(command)) {
        command->execute(world);
    }
}

void ServerGameLoop::update() {
    regen_ticks++;
    bool do_regen = (regen_ticks >= REGEN_EVERY_N_TICKS);
    if (do_regen) regen_ticks = 0;

    for (auto& [id, player] : world.get_players_mutable()) {
        // ── Cooldown de ataque ──
        if (player.attack_cooldown > 0)
            player.attack_cooldown--;

        // ── Meditación: regenerar MP por tick ──
        if (player.meditating && !player.is_ghost &&
            static_cast<Class>(player.cls) != Class::WARRIOR) {
            player.mp = std::min(player.max_mp,
                static_cast<uint16_t>(player.mp + (player.intelligence / 10 + 1)));
        }

        // ── Regeneración natural (por tiempo) ──
        // Solo cada REGEN_EVERY_N_TICKS ticks (1 vez por segundo a 30Hz)
        if (do_regen && !player.is_ghost) {
            // BUG FIX #6: regeneración de HP más lenta
            if (player.hp < player.max_hp)
                player.hp = std::min(player.max_hp,
                    static_cast<uint16_t>(player.hp + hp_regen_per_interval(player)));

            // MP natural (sin meditación)
            if (!player.meditating && player.mp < player.max_mp &&
                static_cast<Class>(player.cls) != Class::WARRIOR)
                player.mp = std::min(player.max_mp,
                    static_cast<uint16_t>(player.mp + mp_regen_per_interval(player)));
        }
    }

    world.tick_npcs(tick);
    world.cleanup_items(tick);
}

void ServerGameLoop::broadcast_snapshots() {
    auto entities = world.get_entities();
    const auto& players = world.get_players();

    for (const auto& [client_id, player] : players) {
        SnapshotDTO snap = world.build_snapshot(client_id, tick, entities);
        queue_monitor.send_to(client_id, snap);
    }
}

uint16_t ServerGameLoop::hp_regen_per_interval(const PlayerData& p) {
    // BUG FIX #6: reducida la regeneración de HP.
    // Antes devolvía hasta 2-3 HP/seg (factor * 2.0).
    // Ahora devuelve siempre 1 HP/seg, diferenciado levemente por raza.
    // A 30 ticks/seg con REGEN_EVERY_N_TICKS = 30, esto equivale a 1 HP/seg.
    float factor = 1.0f;
    switch (static_cast<Race>(p.race)) {
        case Race::HUMAN: factor = 1.0f; break;
        case Race::ELF:   factor = 0.7f; break;  // elfos se recuperan más lento
        case Race::DWARF: factor = 1.3f; break;  // enanos se recuperan más rápido (pero sigue siendo ~1 HP/seg)
        case Race::GNOME: factor = 0.9f; break;
    }
    // Máximo 1 HP por intervalo para la mayoría de razas
    // (el enano llega a 1, ya que floor(1.3) = 1 también con max 1)
    return static_cast<uint16_t>(std::max(1.0f, std::floor(factor)));
}

uint16_t ServerGameLoop::mp_regen_per_interval(const PlayerData& p) {
    float factor = 1.0f;
    switch (static_cast<Race>(p.race)) {
        case Race::HUMAN: factor = 1.0f; break;
        case Race::ELF:   factor = 1.2f; break;
        case Race::DWARF: factor = 0.7f; break;
        case Race::GNOME: factor = 1.4f; break;
    }
    return static_cast<uint16_t>(std::max(1.0f, factor * 1.5f));
}


void ServerGameLoop::save_players() {
    const auto& players = world.get_players();
    for (const auto& [client_id, player] : players) {
        save_queue.push(player);
    }
}
