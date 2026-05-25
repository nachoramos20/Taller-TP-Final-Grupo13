#include <iostream>
#include <string>
#include "net/Acceptor.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    try {
        std::string port = argv[1];
        Acceptor acceptor(port);

        std::cout << "Servidor escuchando en puerto " << port << "\n";
        std::cout << "Presioná 'q' + Enter para cerrar\n";

        acceptor.start();

        char c;
        while (std::cin >> c && c != 'q') {}

        std::cout << "Cerrando servidor...\n";
        acceptor.stop();
        acceptor.join();

    } catch (const std::exception& e) {
        std::cerr << "Error fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}