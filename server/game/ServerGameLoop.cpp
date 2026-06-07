#include "ServerGameLoop.h"
#include "PlayerData.h"
#include "Stats.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

ServerGameLoop::ServerGameLoop(Queue<std::shared_ptr<ServerCommand>>& command_queue,
                               QueueMonitor& queue_monitor, Queue<PlayerData>& save_queue)
    : command_queue(command_queue),
      queue_monitor(queue_monitor),
      save_queue(save_queue),
      world(100, 100), tick(0),
      regen_ticks(0) {}

void ServerGameLoop::run() {
    using clock = std::chrono::steady_clock;
    auto next_tick = clock::now();

    // Spawnear algunos NPCs iniciales
    world.spawn_npc(NpcId::GOBLIN,   20, 20);
    world.spawn_npc(NpcId::SKELETON, 25, 25);
    world.spawn_npc(NpcId::ZOMBIE,   30, 30);
    world.spawn_npc(NpcId::SPIDER,   15, 35);
    world.spawn_npc(NpcId::ORC,      40, 40);

    while (should_keep_running()) {
        process_commands();
        update();
        broadcast_snapshots();
        world.clear_broadcast_messages();

        tick++;
        if (tick % 60 == 0) {
            save_players();
        }
        next_tick += std::chrono::milliseconds(SERVER_TICK_MS);
        std::this_thread::sleep_until(next_tick);
    }
}

void ServerGameLoop::stop() { Thread::stop(); }

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
        // Mana = FClaseMeditacion * Inteligencia * segundos
        // Aproximamos por tick: MP += intel/10+1
        if (player.meditating && !player.is_ghost &&
            static_cast<Class>(player.cls) != Class::WARRIOR) {
            player.mp = std::min(player.max_mp,
                static_cast<uint16_t>(player.mp + (player.intelligence / 10 + 1)));
        }

        // ── Regeneración natural (por tiempo) ──
        // Solo cada REGEN_EVERY_N_TICKS ticks
        if (do_regen && !player.is_ghost) {
            // HP: Vida = FRazaRecuperacion * segundos  (1 HP/seg aprox)
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

    world.tick_npcs();
    cleanup_dead_npcs();
}

void ServerGameLoop::broadcast_snapshots() {
    auto entities = world.get_entities();
    const auto& players = world.get_players();

    for (const auto& [client_id, player] : players) {
        SnapshotDTO snap = world.build_snapshot(client_id, tick, entities);
        queue_monitor.send_to(client_id, snap);
    }
}

void ServerGameLoop::cleanup_dead_npcs() {
    auto& npcs = const_cast<std::vector<NpcData>&>(world.get_npcs());
    npcs.erase(
        std::remove_if(npcs.begin(), npcs.end(),
            [](const NpcData& n){ return n.hp == 0; }),
        npcs.end());
}

uint16_t ServerGameLoop::hp_regen_per_interval(const PlayerData& p) {
    // FRazaRecuperacion diferenciado por raza (aprox)
    float factor = 1.0f;
    switch (static_cast<Race>(p.race)) {
        case Race::HUMAN: factor = 1.0f; break;
        case Race::ELF:   factor = 0.8f; break;
        case Race::DWARF: factor = 1.3f; break;
        case Race::GNOME: factor = 1.1f; break;
    }
    // ~1 HP por segundo; REGEN_EVERY_N_TICKS ticks = 1 seg (a 30Hz)
    return static_cast<uint16_t>(std::max(1.0f, factor * 2.0f));
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