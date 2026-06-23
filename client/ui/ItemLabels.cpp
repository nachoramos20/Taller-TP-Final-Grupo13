#include "ItemLabels.h"
#include <unordered_map>
#include <vector>
#include "../../common/protocol/protocol.h"

// Patrón aplicado: Factory + tabla de lookup (en vez de 2 switches de 26
// casos + una cadena de 8 if/else por rango) para esta metadata. Agregar
// un item nuevo es agregar una fila a cada tabla, no un case más en cada
// switch.
namespace {
    const std::unordered_map<uint8_t, const char*>& name_table() {
        using I = ItemId;
        static const std::unordered_map<uint8_t, const char*> table = {
            {static_cast<uint8_t>(I::SWORD),               "Espada"},
            {static_cast<uint8_t>(I::AXE),                 "Hacha"},
            {static_cast<uint8_t>(I::HAMMER),              "Martillo"},
            {static_cast<uint8_t>(I::SIMPLE_BOW),          "Arco Simple"},
            {static_cast<uint8_t>(I::COMPOUND_BOW),        "Arco Compuesto"},
            {static_cast<uint8_t>(I::ELVEN_FLUTE),         "Flauta Élfica"},
            {static_cast<uint8_t>(I::ASH_STICK),           "Vara de Fresno"},
            {static_cast<uint8_t>(I::NUDOSO_STAFF),        "Báculo Nudoso"},
            {static_cast<uint8_t>(I::GEMMED_STAFF),        "Báculo Engarzado"},
            {static_cast<uint8_t>(I::LEATHER_ARMOR),       "Túnica de Clérigo"},
            {static_cast<uint8_t>(I::CLERIC_BLACK_ARMOR),  "Túnica Negra"},
            {static_cast<uint8_t>(I::MAGE_COMMON_ARMOR),   "Ropa de Mago"},
            {static_cast<uint8_t>(I::MAGE_ROYAL_ARMOR),    "Mago Real"},
            {static_cast<uint8_t>(I::PLATE_ARMOR),         "Guerrero Ejecutor"},
            {static_cast<uint8_t>(I::WARRIOR_EPIC_ARMOR),  "Guerrero Épico"},
            {static_cast<uint8_t>(I::PALADIN_MAGIC_ARMOR), "Paladín Mágico"},
            {static_cast<uint8_t>(I::PALADIN_ROYAL_ARMOR), "Paladín Real"},
            {static_cast<uint8_t>(I::HOOD),                "Capucha"},
            {static_cast<uint8_t>(I::IRON_HELMET),         "Casco de Hierro"},
            {static_cast<uint8_t>(I::MAGIC_HAT),           "Sombrero Mágico"},
            {static_cast<uint8_t>(I::TURTLE_SHIELD),       "Escudo Tortuga"},
            {static_cast<uint8_t>(I::IRON_SHIELD),         "Escudo de Hierro"},
            {static_cast<uint8_t>(I::BOCA_SHIELD),         "Escudo Boca"},
            {static_cast<uint8_t>(I::HEALTH_POTION),       "Poción de Vida"},
            {static_cast<uint8_t>(I::MANA_POTION),         "Poción de Maná"},
            {static_cast<uint8_t>(I::GOLD_PILE),           "Monedas de Oro"},
        };
        return table;
    }

    const std::unordered_map<uint8_t, const char*>& abbr_table() {
        using I = ItemId;
        static const std::unordered_map<uint8_t, const char*> table = {
            {static_cast<uint8_t>(I::SWORD),               "ESP"},
            {static_cast<uint8_t>(I::AXE),                 "HAC"},
            {static_cast<uint8_t>(I::HAMMER),              "MAR"},
            {static_cast<uint8_t>(I::SIMPLE_BOW),          "ARS"},
            {static_cast<uint8_t>(I::COMPOUND_BOW),        "ARC"},
            {static_cast<uint8_t>(I::ELVEN_FLUTE),         "FLA"},
            {static_cast<uint8_t>(I::ASH_STICK),           "VFR"},
            {static_cast<uint8_t>(I::NUDOSO_STAFF),        "BNU"},
            {static_cast<uint8_t>(I::GEMMED_STAFF),        "BAE"},
            {static_cast<uint8_t>(I::LEATHER_ARMOR),       "TCL"},
            {static_cast<uint8_t>(I::CLERIC_BLACK_ARMOR),  "TNE"},
            {static_cast<uint8_t>(I::MAGE_COMMON_ARMOR),   "RMA"},
            {static_cast<uint8_t>(I::MAGE_ROYAL_ARMOR),    "RMR"},
            {static_cast<uint8_t>(I::PLATE_ARMOR),         "GEJ"},
            {static_cast<uint8_t>(I::WARRIOR_EPIC_ARMOR),  "GEP"},
            {static_cast<uint8_t>(I::PALADIN_MAGIC_ARMOR), "PMA"},
            {static_cast<uint8_t>(I::PALADIN_ROYAL_ARMOR), "PRE"},
            {static_cast<uint8_t>(I::HOOD),                "CAP"},
            {static_cast<uint8_t>(I::IRON_HELMET),         "CHI"},
            {static_cast<uint8_t>(I::MAGIC_HAT),           "SOM"},
            {static_cast<uint8_t>(I::TURTLE_SHIELD),       "EST"},
            {static_cast<uint8_t>(I::IRON_SHIELD),         "ESI"},
            {static_cast<uint8_t>(I::BOCA_SHIELD),         "ESB"},
            {static_cast<uint8_t>(I::HEALTH_POTION),       "PVI"},
            {static_cast<uint8_t>(I::MANA_POTION),         "PMA"},
            {static_cast<uint8_t>(I::GOLD_PILE),           "ORO"},
        };
        return table;
    }

    struct KindRange { uint8_t lo, hi; const char* label; };

    const std::vector<KindRange>& kind_ranges() {
        using I = ItemId;
        static const std::vector<KindRange> ranges = {
            {static_cast<uint8_t>(I::SWORD),          static_cast<uint8_t>(I::HAMMER),             "Arma Melee"},
            {static_cast<uint8_t>(I::SIMPLE_BOW),     static_cast<uint8_t>(I::COMPOUND_BOW),       "Arma Ranged"},
            {static_cast<uint8_t>(I::ELVEN_FLUTE),    static_cast<uint8_t>(I::NUDOSO_STAFF),       "Arma Mágica"},
            {static_cast<uint8_t>(I::LEATHER_ARMOR),  static_cast<uint8_t>(I::PALADIN_ROYAL_ARMOR),"Armadura"},
            {static_cast<uint8_t>(I::HOOD),           static_cast<uint8_t>(I::MAGIC_HAT),          "Casco"},
            {static_cast<uint8_t>(I::TURTLE_SHIELD),  static_cast<uint8_t>(I::BOCA_SHIELD),        "Escudo"},
            {static_cast<uint8_t>(I::HEALTH_POTION),  static_cast<uint8_t>(I::MANA_POTION),        "Poción"},
            {static_cast<uint8_t>(I::GOLD_PILE),      static_cast<uint8_t>(I::GOLD_PILE),          "Oro"},
        };
        return ranges;
    }
}

const char* ItemLabels::name(uint8_t id) {
    auto it = name_table().find(id);
    return it != name_table().end() ? it->second : nullptr;
}

const char* ItemLabels::abbr(uint8_t id) {
    auto it = abbr_table().find(id);
    return it != abbr_table().end() ? it->second : "???";
}

const char* ItemLabels::kind(uint8_t id) {
    if (id == 0) return nullptr;
    for (const KindRange& range : kind_ranges())
        if (id >= range.lo && id <= range.hi) return range.label;
    return nullptr;
}
