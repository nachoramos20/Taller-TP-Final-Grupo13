#include "ConfigLoader.h"

#include "AudioConfig.h"
#include "ClientConfig.h"
#include "ItemVisualConfig.h"
#include "NpcVisualConfig.h"
#include "RacesClassesConfig.h"
#include "SpellVfxConfig.h"

namespace {

bool load_config_file(bool load_ok, const char* path, std::string& error_message) {
    if (load_ok) {
        return true;
    }

    error_message = std::string("Error: no se pudo cargar ") + path;
    return false;
}

}  // namespace

bool ConfigLoader::load_all(std::string& error_message) {
    error_message.clear();

    if (!load_config_file(ClientConfig::instance().load("config/client_config.toml"),
                          "config/client_config.toml", error_message)) {
        return false;
    }
    if (!load_config_file(AudioConfig::instance().load("config/audio_config.toml"),
                          "config/audio_config.toml", error_message)) {
        return false;
    }
    if (!load_config_file(SpellVfxConfig::instance().load("config/spells_vfx.toml"),
                          "config/spells_vfx.toml", error_message)) {
        return false;
    }
    if (!load_config_file(RacesClassesConfig::instance().load("config/races_classes.toml"),
                          "config/races_classes.toml", error_message)) {
        return false;
    }
    if (!load_config_file(ItemVisualConfig::instance().load("config/item_visuals.toml"),
                          "config/item_visuals.toml", error_message)) {
        return false;
    }
    if (!load_config_file(NpcVisualConfig::instance().load("config/npc_visuals.toml"),
                          "config/npc_visuals.toml", error_message)) {
        return false;
    }

    return true;
}
