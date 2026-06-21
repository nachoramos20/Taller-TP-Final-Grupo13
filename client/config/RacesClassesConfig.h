#ifndef RACES_CLASSES_CONFIG_H
#define RACES_CLASSES_CONFIG_H

#include <string>
#include <unordered_map>
#include <cstdint>

class RacesClassesConfig {
public:
    // Estructura para una raza
    struct Race {
        uint8_t id;
        std::string name;
        std::string sprite_path;
        std::string description;
    };

    // Estructura para una clase
    struct Class {
        uint8_t id;
        std::string name;
        std::string flavor;
        std::string benefits;
    };

    // Estructura para mensajes de login
    struct LoginMessages {
        std::string welcome;
        std::string connected;
        std::string invalid_user;
    };

    // Singleton
    static RacesClassesConfig& instance();

    // Cargar configuración desde TOML
    bool load(const std::string& config_path);

    // Getters
    const Race& get_race(uint8_t race_id) const;
    const Class& get_class(uint8_t class_id) const;
    const LoginMessages& get_login_messages() const;

    // Iteradores/acceso
    const std::unordered_map<uint8_t, Race>& get_races() const;
    const std::unordered_map<uint8_t, Class>& get_classes() const;

private:
    std::unordered_map<uint8_t, Race> races;
    std::unordered_map<uint8_t, Class> classes;
    LoginMessages login_messages;

    // Valores por defecto
    static const Race DEFAULT_RACE;
    static const Class DEFAULT_CLASS;
    static const LoginMessages DEFAULT_LOGIN_MESSAGES;

    RacesClassesConfig() = default;
    ~RacesClassesConfig() = default;

    // Prevenir copia
    RacesClassesConfig(const RacesClassesConfig&) = delete;
    RacesClassesConfig& operator=(const RacesClassesConfig&) = delete;
};

#endif // RACES_CLASSES_CONFIG_H
