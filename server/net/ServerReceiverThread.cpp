#include "ServerReceiverThread.h"
#include <iostream>
#include <memory>

ServerReceiverThread::ServerReceiverThread(uint16_t client_id,
                                           Queue<std::shared_ptr<ServerCommand>>& command_queue,
                                           QueueMonitor& queue_monitor,
                                           Queue<SnapshotDTO>& sender_queue,
                                           std::atomic<bool>& client_alive,
                                           ServerProtocol& server_protocol,
                                           PersistenceMonitor& persistence_monitor)
        : client_id(client_id),
            server_protocol(server_protocol),
            command_queue(command_queue),
            queue_monitor(queue_monitor),
            sender_queue(sender_queue),
            client_alive(client_alive),
            persistence_monitor(persistence_monitor) {}

TileDTO get_tile(MapaDTO& mapa, uint16_t x, uint16_t y) {
    return mapa.tiles[y * mapa.width + x];
}

void build_edificio(MapaDTO& mapa, uint16_t x, uint16_t y) {
    TileDTO& tile_inf_izq = mapa.tiles[y * mapa.width + x];
    tile_inf_izq.floor_id = 1;
    tile_inf_izq.object_id = 1;
    tile_inf_izq.object_superior_id = 0;


    TileDTO& tile_inf_der = mapa.tiles[y * mapa.width + (x + 1)];
    tile_inf_der.floor_id = 1;
    tile_inf_der.object_id = 1;
    tile_inf_der.object_superior_id = 0;

    TileDTO& tile_sup_izq = mapa.tiles[(y + 1) * mapa.width + x];
    tile_sup_izq.floor_id = 1;
    tile_sup_izq.object_id = 0;
    tile_sup_izq.object_superior_id = 2;

    TileDTO& tile_sup_izq2 = mapa.tiles[(y + 2) * mapa.width + x];
    tile_sup_izq2.floor_id = 1;
    tile_sup_izq2.object_id = 0;
    tile_sup_izq2.object_superior_id = 2;

    TileDTO& tile_sup_der = mapa.tiles[(y + 1) * mapa.width + (x + 1)];
    tile_sup_der.floor_id = 1;
    tile_sup_der.object_id = 0;
    tile_sup_der.object_superior_id = 2;

    TileDTO& tile_sup_der2 = mapa.tiles[(y + 2) * mapa.width + (x + 1)];
    tile_sup_der2.floor_id = 1;
    tile_sup_der2.object_id = 0;
    tile_sup_der2.object_superior_id = 2;

}
MapaDTO build_mapa_inicial() {
    MapaDTO mapa{};
    mapa.width = 100;
    mapa.height = 100;
    mapa.tiles.resize(mapa.width * mapa.height);
    for (int i = 0; i < mapa.width * mapa.height; i++) {
        mapa.tiles[i].floor_id = 1;
        mapa.tiles[i].object_id = 0;
        mapa.tiles[i].object_superior_id = 0;
    }

    build_edificio(mapa, 10, 10);
    build_edificio(mapa, 20, 20);

    return mapa;
}
void ServerReceiverThread::run() {
    try {
        std::string username = this->server_protocol.handshake();
        
        PlayerData player_data;
        persistence_monitor.login_or_register(username, player_data, this->client_id);
        
        server_protocol.send_login_ok(this->client_id);
        MapaDTO mapa = build_mapa_inicial();
        server_protocol.send_mapa(mapa);

        queue_monitor.add(this->client_id, &this->sender_queue);
        this->command_queue.push(std::make_shared<LoginCommand>(player_data));

        while (should_keep_running()) {
            std::shared_ptr<ServerCommand> cmd = server_protocol.receive_command(this->client_id);
            if (cmd) {
                this->command_queue.push(cmd);
            } else {
                this->client_alive = false;
                break;
            }
        }
    } catch (const ClosedQueue&) {
    } catch (const std::exception& e) {
        std::cerr << "Cliente " << client_id
                  << " desconectado: " << e.what() << "\n";
    }

    client_alive = false;
}

void ServerReceiverThread::stop() {
    client_alive = false;
    Thread::stop();
}