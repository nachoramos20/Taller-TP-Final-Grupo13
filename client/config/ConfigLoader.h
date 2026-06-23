#pragma once

#include <string>

// Carga en secuencia todos los singletons de configuración del cliente
// (ClientConfig, AudioConfig, SpellVfxConfig, RacesClassesConfig,
// ItemVisualConfig, NpcVisualConfig) desde sus .toml. Falla temprano: si
// alguno no carga, devuelve false con el path que falló en error_message
// y no sigue con el resto.
class ConfigLoader {
public:
    static bool load_all(std::string& error_message);
};
