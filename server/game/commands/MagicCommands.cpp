#include "Commands.h"
#include <algorithm>

MeditateCommand::MeditateCommand(uint16_t c) : client_id(c) {}

void MeditateCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || p->is_ghost) return;

    if (static_cast<Class>(p->cls) == Class::WARRIOR) {
        world.push_message(client_id, 0, "El Guerrero no puede meditar.");
        return;
    }

    p->meditating = !p->meditating;
    if (p->meditating)
        world.push_message(client_id, 0, "Entraste en meditación.");
    else
        world.push_message(client_id, 0, "Saliste de la meditación.");
}

ResurrectCommand::ResurrectCommand(uint16_t c) : client_id(c) {}

void ResurrectCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p || !p->is_ghost) return;

    world.update_occupied({p->pos_x, p->pos_y}, false);
    p->pos_x      = 5;
    p->pos_y      = 5;
    p->is_ghost   = false;
    p->meditating = false;
    p->hp         = p->max_hp / 4;
    p->mp         = 0;
    world.update_occupied({p->pos_x, p->pos_y}, true);
    world.push_message(client_id, 0, "Resucitaste junto al sanador.");
}
