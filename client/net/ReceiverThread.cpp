#include "ReceiverThread.h"

#include <iostream>
#include <string>

#include <SDL2/SDL.h>

ReceiverThread::ReceiverThread(ClientProtocol& protocol, Queue<SnapshotDTO>& snapshot_queue,
                               Queue<MapaDTO>& map_queue, std::atomic<bool>& connected):
        _protocol(protocol),
        _snapshot_queue(snapshot_queue),
        _map_queue(map_queue),
        _connected(connected),
        _my_entity_id(0) {}

void ReceiverThread::run() {
    try {
        // ── Fase 1: handshake (LOGIN_OK / LOGIN_ERROR / MAPA) ────────────────
        bool got_ok = false;
        bool got_map = false;

        // No se aplica tabla en este switch ni en el de game_loop_receive:
        // son 2-3 casos fijos (el handshake y el game loop no agregan
        // tipos de mensaje nuevos sin tocar también el protocolo), y cada
        // rama tiene control de flujo propio (mueve banderas locales,
        // hace return) que no encaja en una firma uniforme de tabla.
        while (should_keep_running() && !(got_ok && got_map)) {
            MsgType opcode = _protocol.recv_opcode();
            switch (opcode) {
                case MsgType::LOGIN_OK: {
                    uint16_t eid = _protocol.recv_login_ok();
                    _my_entity_id = eid;
                    std::string msg = "OK:" + std::to_string(eid);
                    _login_ok_queue.push(std::move(msg));
                    got_ok = true;
                    break;
                }
                case MsgType::LOGIN_ERROR: {
                    std::string err = _protocol.recv_login_error();
                    _login_error_queue.push(err);
                    // No paramos: el servidor puede seguir aceptando reintentos
                    // en el mismo socket (el ServerReceiverThread los soporta).
                    // Esperamos un LOGIN_OK posterior.
                    // Pero en la arquitectura actual un error cierra el flujo,
                    // así que devolvemos error y el main manejará la reconexión.
                    _connected = false;
                    return;
                }
                case MsgType::MAPA: {
                    MapaDTO map = _protocol.recv_map();
                    _map_queue.push(std::move(map));
                    got_map = true;
                    break;
                }
                default:
                    break;
            }
        }

        if (!got_ok) {
            _connected = false;
            return;
        }

        // ── Fase 2: game loop ─────────────────────────────────────────────────
        game_loop_receive();

    } catch (const ClosedQueue&) {
    } catch (const std::exception&) {}
    _connected = false;
}

void ReceiverThread::game_loop_receive() {
    try {
        while (should_keep_running()) {
            MsgType opcode = _protocol.recv_opcode();
            switch (opcode) {
                case MsgType::SNAPSHOT: {
                    SnapshotDTO snap = _protocol.recv_snapshot();
                    _snapshot_queue.push(std::move(snap));
                    break;
                }
                case MsgType::MAPA: {
                    MapaDTO map = _protocol.recv_map();
                    _map_queue.push(std::move(map));
                    break;
                }
                default:
                    break;
            }
        }
    } catch (const ClosedQueue&) {
    } catch (const std::exception&) {}
}

void ReceiverThread::stop() { Thread::stop(); }

uint16_t ReceiverThread::my_entity_id() const { return _my_entity_id; }

// Bloquea hasta recibir el resultado del handshake.
HandshakeResult ReceiverThread::wait_handshake(std::string& error_msg) {
    // Intentar ok primero (timeout implícito: el pop bloquea)
    // Usamos try_pop con pequeño spin para no bloquear indefinidamente si
    // el servidor ya respondió error y el hilo terminó.
    for (int attempts = 0; attempts < 3000; attempts++) {
        std::string ok_msg;
        if (_login_ok_queue.try_pop(ok_msg))
            return HandshakeResult::OK;

        std::string err_msg;
        if (_login_error_queue.try_pop(err_msg)) {
            error_msg = err_msg;
            return HandshakeResult::ERROR;
        }

        if (!_connected) {
            error_msg = "Conexion perdida con el servidor.";
            return HandshakeResult::ERROR;
        }

        SDL_Delay(10);
    }
    error_msg = "Timeout esperando respuesta del servidor.";
    return HandshakeResult::ERROR;
}
