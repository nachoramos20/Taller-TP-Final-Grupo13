#include "NetSession.h"

#include <exception>

NetSession::~NetSession() { shutdown(); }

bool NetSession::connect(const std::string& host, const std::string& port,
                         std::string& error_message) {
    error_message.clear();

    try {
        Socket socket(host.c_str(), port.c_str());
        _protocol = std::make_unique<ClientProtocol>(std::move(socket));
    } catch (const std::exception& exception) {
        error_message = std::string("No se pudo conectar: ") + exception.what();
        return false;
    }

    _connected = true;
    start_threads();
    return true;
}

bool NetSession::authenticate(const std::string& username, bool should_register, uint8_t race_id,
                              uint8_t class_id, std::string& error_message) {
    if (!_sender_thread || !_receiver_thread) {
        error_message = "Sesion de red no inicializada.";
        return false;
    }

    if (should_register) {
        _command_queue.push(Command::register_player(username, race_id, class_id));
    } else {
        _command_queue.push(Command::login(username));
    }

    HandshakeResult handshake_result = _receiver_thread->wait_handshake(error_message);
    if (handshake_result == HandshakeResult::OK) {
        return true;
    }

    shutdown();
    return false;
}

void NetSession::shutdown() {
    _connected = false;

    try {
        _command_queue.close();
    } catch (...) {}
    try {
        _snapshot_queue.close();
    } catch (...) {}
    try {
        _map_queue.close();
    } catch (...) {}

    if (_sender_thread) {
        _sender_thread->stop();
    }
    if (_receiver_thread) {
        _receiver_thread->stop();
    }
    if (_sender_thread) {
        _sender_thread->join();
        _sender_thread.reset();
    }
    if (_receiver_thread) {
        _receiver_thread->join();
        _receiver_thread.reset();
    }
    _protocol.reset();
}

void NetSession::start_threads() {
    _sender_thread = std::make_unique<SenderThread>(*_protocol, _command_queue);
    _receiver_thread =
            std::make_unique<ReceiverThread>(*_protocol, _snapshot_queue, _map_queue, _connected);
    _sender_thread->start();
    _receiver_thread->start();
}
