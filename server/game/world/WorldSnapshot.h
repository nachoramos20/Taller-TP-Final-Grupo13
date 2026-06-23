#ifndef WORLD_SNAPSHOT_H
#define WORLD_SNAPSHOT_H

#include <cstdint>
#include <memory>
#include <vector>

#include "../../../common/protocol/dtos.h"

class WorldPlayers;
class WorldNpcs;
class WorldItems;
class WorldChat;

class WorldSnapshot {
private:
    const WorldPlayers& players;
    const WorldNpcs&    npcs;
    const WorldItems&   items;
    WorldChat&          chat; // collect() es mutating
public:
    WorldSnapshot(const WorldPlayers& p, const WorldNpcs& n,
                  const WorldItems& i, WorldChat& c)
        : players(p), npcs(n), items(i), chat(c) {}

    std::shared_ptr<std::vector<EntityDTO>> get_entities() const;

    SnapshotDTO build(uint16_t client_id,
                      uint32_t tick,
                      const std::shared_ptr<std::vector<EntityDTO>>& entities,
                      const std::shared_ptr<std::vector<SpellEventDTO>>& spell_events) const;
};

#endif
