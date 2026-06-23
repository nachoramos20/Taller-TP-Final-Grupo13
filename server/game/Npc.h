#ifndef NPC_H
#define NPC_H

#include <cstdint>
#include <vector>
#include "../../common/protocol/protocol.h"
#include "entities/entity_types.h"

namespace Npcs {
    const NpcTemplate& tpl(NpcId id);
    const std::vector<NpcId>& all_ids();
}

#endif
