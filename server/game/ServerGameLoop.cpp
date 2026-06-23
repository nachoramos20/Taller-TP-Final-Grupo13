#include "ServerGameLoop.h"
#include "entities/PlayerData.h"
#include "Stats.h"
#include "config/GameConfig.h"
#include "world/WorldBuilder.h"

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
      regen_ticks(0) {
    int tick_rate_hz = GameConfig::get().formulas().tick_rate_hz;
    tick_ms = 1000 / tick_rate_hz;
    regen_every_n_ticks = tick_rate_hz;
}

void ServerGameLoop::run() {
    using clock = std::chrono::steady_clock;
    auto next_tick = clock::now();

    WorldBuilder::configure(world);

    while (should_keep_running()) {
        process_commands();
        update();
        broadcast_snapshots();
        world.clear_broadcast_messages();

        tick++;
        if (tick % 60 == 0) save_players();
        next_tick += std::chrono::milliseconds(tick_ms);
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
    bool do_regen = (regen_ticks >= regen_every_n_ticks);
    if (do_regen) regen_ticks = 0;

    for (auto& [id, player] : world.get_players_mutable()) {
        // ── Cooldown de ataque ──
        if (player.is_in_combat())
            player.attack_cooldown--;

        // ── Meditación: regenerar MP por tick ──
        if (player.meditating && !player.is_ghost &&
            static_cast<Class>(player.cls) != Class::WARRIOR) {
            player.mp = std::min(player.max_mp,
                static_cast<uint16_t>(player.mp + (player.intelligence / 10 + 1)));
        }

        // ── Regeneración natural (por tiempo) ──
        // Solo cada regen_every_n_ticks ticks (1 vez por segundo al tick_rate_hz configurado)
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
    float factor = GameConfig::get().race(p.race).hp_regen_factor;
    // Máximo 1 HP por intervalo para la mayoría de razas
    // (el enano llega a 1, ya que floor(1.3) = 1 también con max 1)
    return static_cast<uint16_t>(std::max(1.0f, std::floor(factor)));
}

uint16_t ServerGameLoop::mp_regen_per_interval(const PlayerData& p) {
    float factor = GameConfig::get().race(p.race).mp_regen_factor;
    return static_cast<uint16_t>(std::max(1.0f, factor * 1.5f));
}


void ServerGameLoop::save_players() {
    const auto& players = world.get_players();
    for (const auto& [client_id, player] : players) {
        save_queue.push(player);
    }
}
