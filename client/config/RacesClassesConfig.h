#ifndef RACES_CLASSES_CONFIG_H
#define RACES_CLASSES_CONFIG_H

#include <cstdint>
#include <string>
#include <unordered_map>

// Texto e ícono de cada raza/clase jugable y los mensajes de login,
// cargados desde races_classes.toml — usado por LoginScreen.
class RacesClassesConfig {
public:
    struct Race {
        uint8_t id;
        std::string name;
        std::string sprite_path;
        std::string description;
    };

    struct Class {
        uint8_t id;
        std::string name;
        std::string flavor;
        std::string benefits;
    };

    struct LoginMessages {
        std::string welcome;
        std::string connected;
        std::string invalid_user;
    };

    static RacesClassesConfig& instance();

    bool load(const std::string& config_path);

    const Race& get_race(uint8_t race_id) const;
    const Class& get_class(uint8_t class_id) const;
    const LoginMessages& get_login_messages() const;

    const std::unordered_map<uint8_t, Race>& get_races() const;
    const std::unordered_map<uint8_t, Class>& get_classes() const;

private:
    std::unordered_map<uint8_t, Race> races;
    std::unordered_map<uint8_t, Class> classes;
    LoginMessages login_messages;

    static const Race DEFAULT_RACE;
    static const Class DEFAULT_CLASS;
    static const LoginMessages DEFAULT_LOGIN_MESSAGES;

    RacesClassesConfig() = default;
    ~RacesClassesConfig() = default;

    RacesClassesConfig(const RacesClassesConfig&) = delete;
    RacesClassesConfig& operator=(const RacesClassesConfig&) = delete;
};

#endif  // RACES_CLASSES_CONFIG_H
