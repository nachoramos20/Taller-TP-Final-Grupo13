#include "RacesClassesConfig.h"
#include <toml++/toml.h>
#include <iostream>

const RacesClassesConfig::Race RacesClassesConfig::DEFAULT_RACE = {
    0, "Unknown", "", ""
};

const RacesClassesConfig::Class RacesClassesConfig::DEFAULT_CLASS = {
    0, "Unknown", "", ""
};

const RacesClassesConfig::LoginMessages RacesClassesConfig::DEFAULT_LOGIN_MESSAGES = {
    "Welcome to Argentum Online",
    "Successfully connected!",
    "Invalid username or password"
};

RacesClassesConfig& RacesClassesConfig::instance() {
    static RacesClassesConfig instance;
    return instance;
}

bool RacesClassesConfig::load(const std::string& config_path) {
    try {
        auto config = toml::parse_file(config_path);

        for (int i = 1; i <= 4; i++) {
            if (auto race_table = config["race"][std::to_string(i)]; race_table) {
                Race race;
                race.id = race_table["id"].value_or(static_cast<uint8_t>(i));
                race.name = race_table["name"].value_or(std::string(""));
                race.sprite_path = race_table["sprite_path"].value_or(std::string(""));
                race.description = race_table["description"].value_or(std::string(""));
                races[race.id] = race;
            }
        }

        for (int i = 1; i <= 4; i++) {
            if (auto class_table = config["class"][std::to_string(i)]; class_table) {
                Class cls;
                cls.id = class_table["id"].value_or(static_cast<uint8_t>(i));
                cls.name = class_table["name"].value_or(std::string(""));
                cls.flavor = class_table["flavor"].value_or(std::string(""));
                cls.benefits = class_table["benefits"].value_or(std::string(""));
                classes[cls.id] = cls;
            }
        }

        if (auto messages_table = config["login_messages"]) {
            login_messages.welcome = messages_table["welcome"].value_or(std::string("Welcome to Argentum Online"));
            login_messages.connected = messages_table["connected"].value_or(std::string("Successfully connected!"));
            login_messages.invalid_user = messages_table["invalid_user"].value_or(std::string("Invalid username or password"));
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[RacesClassesConfig] Error cargando configuración: " << e.what() << std::endl;
        return false;
    }
}

const RacesClassesConfig::Race& RacesClassesConfig::get_race(uint8_t race_id) const {
    auto it = races.find(race_id);
    if (it != races.end()) {
        return it->second;
    }
    return DEFAULT_RACE;
}

const RacesClassesConfig::Class& RacesClassesConfig::get_class(uint8_t class_id) const {
    auto it = classes.find(class_id);
    if (it != classes.end()) {
        return it->second;
    }
    return DEFAULT_CLASS;
}

const RacesClassesConfig::LoginMessages& RacesClassesConfig::get_login_messages() const {
    return login_messages;
}

const std::unordered_map<uint8_t, RacesClassesConfig::Race>& RacesClassesConfig::get_races() const {
    return races;
}

const std::unordered_map<uint8_t, RacesClassesConfig::Class>& RacesClassesConfig::get_classes() const {
    return classes;
}
