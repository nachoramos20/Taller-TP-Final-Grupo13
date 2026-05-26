#include "ServerProtocol.h"

#include <arpa/inet.h>
#include <utility>

ServerProtocol::ServerProtocol(Socket&& socket)
    : socket(std::move(socket)) {}

void ServerProtocol::send_uint8(uint8_t value) {
    socket.sendall(&value, sizeof(value));
}

void ServerProtocol::send_uint16(uint16_t value) {
    uint16_t net_value = htons(value);
    socket.sendall(&net_value, sizeof(net_value));
}

void ServerProtocol::send_uint32(uint32_t value) {
    uint32_t net_value = htonl(value);
    socket.sendall(&net_value, sizeof(net_value));
}

void ServerProtocol::send_str8(const std::string& s) {
    uint8_t len = static_cast<uint8_t>(s.size());
    send_uint8(len);
    socket.sendall(s.data(), len);
}

