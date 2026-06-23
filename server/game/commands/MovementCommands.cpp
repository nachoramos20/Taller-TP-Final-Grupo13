#include "Commands.h"

MoveCommand::MoveCommand(uint16_t c, uint16_t x, uint16_t y): client_id(c), new_x(x), new_y(y) {}

void MoveCommand::execute(World& world) { world.move_player(client_id, new_x, new_y); }
