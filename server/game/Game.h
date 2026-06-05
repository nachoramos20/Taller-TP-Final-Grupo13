#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <memory>
#include <vector>

#include "../../common/protocol/dtos.h"
#include "PlayerData.h"
#include "World.h"

class Game {
private:
    World world;

public:
    Game();

    // Pass-through al mundo
    void add_player(const PlayerData& player_data);
    void remove_player(uint16_t client_id);
    void move_player(uint16_t client_id, uint16_t new_x, uint16_t new_y);
    void revisar_colisiones();

    const std::unordered_map<uint16_t, PlayerData>& get_players() const;
    std::shared_ptr<std::vector<EntityDTO>> get_entities() const;
    void player_attack(uint16_t client_id, uint16_t target_id);
    void player_equip(uint16_t client_id, uint8_t inv_slot);
    void player_unequip(uint16_t client_id, EquipSlot slot);
    void player_drop(uint16_t client_id, uint8_t inv_slot);
    void player_pick(uint16_t client_id);
    void player_use(uint16_t client_id, uint8_t inv_slot);
    void player_meditate(uint16_t client_id);
    void player_resurrect(uint16_t client_id);


    // Snapshot por cliente (estado personal + entidades visibles)
    SnapshotDTO build_snapshot(uint16_t client_id,
                               uint32_t tick,
                               const std::shared_ptr<std::vector<EntityDTO>>& entities) const;
};

#endif // GAME_H
