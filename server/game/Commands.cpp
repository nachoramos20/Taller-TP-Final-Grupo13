#include "Commands.h"

MoveCommand::MoveCommand(uint16_t c, uint16_t x, uint16_t y) : client_id(c), new_x(x), new_y(y) {}
void MoveCommand::execute(Game& g) { g.move_player(client_id, new_x, new_y); }

LoginCommand::LoginCommand(PlayerData p) : player_data(std::move(p)) {}
void LoginCommand::execute(Game& g) { g.add_player(player_data); }
const std::string& LoginCommand::get_username() const { return player_data.username; }

AttackCommand::AttackCommand(uint16_t c, uint16_t t) : client_id(c), target_id(t) {}
void AttackCommand::execute(Game& g) { g.player_attack(client_id, target_id); }

EquipCommand::EquipCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}
void EquipCommand::execute(Game& g) { g.player_equip(client_id, inv_slot); }

UnequipCommand::UnequipCommand(uint16_t c, EquipSlot s) : client_id(c), slot(s) {}
void UnequipCommand::execute(Game& g) { g.player_unequip(client_id, slot); }

DropCommand::DropCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}
void DropCommand::execute(Game& g) { g.player_drop(client_id, inv_slot); }

PickCommand::PickCommand(uint16_t c) : client_id(c) {}
void PickCommand::execute(Game& g) { g.player_pick(client_id); }

UseItemCommand::UseItemCommand(uint16_t c, uint8_t s) : client_id(c), inv_slot(s) {}
void UseItemCommand::execute(Game& g) { g.player_use(client_id, inv_slot); }

MeditateCommand::MeditateCommand(uint16_t c) : client_id(c) {}
void MeditateCommand::execute(Game& g) { g.player_meditate(client_id); }

ResurrectCommand::ResurrectCommand(uint16_t c) : client_id(c) {}
void ResurrectCommand::execute(Game& g) { g.player_resurrect(client_id); }

LogoutCommand::LogoutCommand(uint16_t c) : client_id(c) {}
void LogoutCommand::execute(Game& g) { g.remove_player(client_id); }
