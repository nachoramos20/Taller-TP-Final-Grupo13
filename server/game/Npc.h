#ifndef NPC_H
#define NPC_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../../common/protocol/protocol.h"
#include "entities/entity_types.h"

// Acceso de solo lectura a las plantillas de NPC definidas en npcs.toml
// (vía GameConfig), cacheadas en NpcTemplate la primera vez que se piden.
namespace Npcs {
const NpcTemplate& tpl(NpcId id);
const std::vector<NpcId>& all_ids();
}  // namespace Npcs

#endif
