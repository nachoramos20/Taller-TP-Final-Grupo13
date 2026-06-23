#include "Protocol.h"

#include <stdexcept>
#include <utility>

Protocol::Protocol(Socket&& socket) : _socket(std::move(socket)) {}

uint8_t Protocol::recv_uint8() {
    uint8_t value;
    int received = _socket.recvall(&value, 1);
    if (received == 0)
        throw std::runtime_error("connection closed");
    return value;
}

uint16_t Protocol::recv_uint16() {
    uint8_t buf[2];
    _socket.recvall(buf, 2);
    return (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
}

uint32_t Protocol::recv_uint32() {
    uint8_t buf[4];
    _socket.recvall(buf, 4);
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) <<  8) |
            static_cast<uint32_t>(buf[3]);
}

std::string Protocol::recv_string() {
    uint8_t len = recv_uint8();
    std::string text(len, '\0');
    _socket.recvall(text.data(), len);
    return text;
}

void Protocol::send_uint8(uint8_t value) {
    _socket.sendall(&value, 1);
}

void Protocol::send_uint16(uint16_t value) {
    uint8_t buf[2] = {
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value & 0xFF)
    };
    _socket.sendall(buf, 2);
}

void Protocol::send_uint32(uint32_t value) {
    uint8_t buf[4] = {
        static_cast<uint8_t>(value >> 24),
        static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value & 0xFF)
    };
    _socket.sendall(buf, 4);
}

void Protocol::send_string(const std::string& text) {
    uint8_t len = static_cast<uint8_t>(text.size());
    send_uint8(len);
    _socket.sendall(text.data(), len);
}

void Protocol::shutdown(int how) {
    _socket.shutdown(how);
}
