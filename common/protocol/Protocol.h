#pragma once

#include <cstdint>
#include <string>
#include "../socket.h"

// Primitivas de framing del protocolo (wire format big-endian) sobre un
// Socket. ServerProtocol y ClientProtocol heredan de esta clase para no
// duplicar la lectura/escritura de enteros y strings de cada lado de la
// conexión; cada una agrega encima sus propios métodos públicos de alto
// nivel (un mensaje por tipo) usando estas primitivas.
//
// El socket se recibe por movimiento en el constructor: `Protocol` (y por
// lo tanto sus hijas) son dueñas exclusivas de la conexión.
class Protocol {
public:
    void shutdown(int how);

    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;
    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = default;
    virtual ~Protocol() = default;

protected:
    explicit Protocol(Socket&& socket);

    uint8_t  recv_uint8();
    uint16_t recv_uint16();
    uint32_t recv_uint32();

    // String length-prefixed: 1 byte de longitud + ese tantos bytes.
    std::string recv_string();

    void send_uint8(uint8_t value);
    void send_uint16(uint16_t value);
    void send_uint32(uint32_t value);
    void send_string(const std::string& text);

    // Acceso directo al socket subyacente, para los pocos casos que no
    // encajan en las primitivas de arriba (p.ej. mandar/recibir un buffer
    // de tiles ya empaquetado de una sola vez).
    Socket _socket;
};
