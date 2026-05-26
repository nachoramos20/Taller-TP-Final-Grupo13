#include "World.h"
#include <cstdlib>
#include "../../common/protocol/protocol.h"

World::World(uint16_t width, uint16_t height) : width(width), height(height) {
    for (uint16_t x = 0; x < width; ++x) {
        for (uint16_t y = 0; y < height; ++y) {
            occupied_positions[std::make_pair(x, y)] = false;
        }
    }
}

void World::move_player(PlayerData& player, const std::pair<uint16_t, uint16_t>& new_pos) {
    if (new_pos.first >= this->width || new_pos.second >= this->height) return;

    const int delta_x = static_cast<int>(new_pos.first) - static_cast<int>(player.pos_x);
    const int delta_y = static_cast<int>(new_pos.second) - static_cast<int>(player.pos_y);
    if (std::abs(delta_x) + std::abs(delta_y) != 1) return;

    
    if (delta_x == 1)
    player.direction = static_cast<uint8_t>(MoveDirection::EAST);
    else if (delta_x == -1)
    player.direction = static_cast<uint8_t>(MoveDirection::WEST);
    else if (delta_y == 1)
    player.direction = static_cast<uint8_t>(MoveDirection::SOUTH);
    else if (delta_y == -1)
    player.direction = static_cast<uint8_t>(MoveDirection::NORTH);

    if (occupied_positions[new_pos]) return;

    occupied_positions[std::make_pair(player.pos_x, player.pos_y)] = false;
    occupied_positions[new_pos] = true;

    player.pos_x = new_pos.first;
    player.pos_y = new_pos.second;
}

void World::update_occupied(const std::pair<uint16_t, uint16_t>& pos, bool occupied) {
    if (pos.first >= this->width || pos.second >= this->height) return;
    occupied_positions[pos] = occupied;
}