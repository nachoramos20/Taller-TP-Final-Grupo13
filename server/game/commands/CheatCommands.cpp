#include "Commands.h"
#include <cstring>

CheatCommand::CheatCommand(uint16_t c, uint8_t cheat_id) : client_id(c), cheat_id(cheat_id) {}

void CheatCommand::execute(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p) return;

    // No se aplica tabla/Factory: son 4 cheats de debug fijos, cada uno con
    // lógica propia (no es una simple asignación de datos), y no es un
    // catálogo pensado para crecer dinámicamente.
    switch (static_cast<CheatId>(cheat_id)) {
        case CheatId::INFINITE_HP: {
            p->cheat_infinite_hp = !p->cheat_infinite_hp;
            if (p->cheat_infinite_hp) p->hp = p->max_hp;
            world.push_message(client_id, 0, p->cheat_infinite_hp
                ? "CHEAT: vida infinita activada."
                : "CHEAT: vida infinita desactivada.");
            break;
        }
        case CheatId::INFINITE_MP: {
            p->cheat_infinite_mp = !p->cheat_infinite_mp;
            if (p->cheat_infinite_mp) p->mp = p->max_mp;
            world.push_message(client_id, 0, p->cheat_infinite_mp
                ? "CHEAT: mana infinito activado."
                : "CHEAT: mana infinito desactivado.");
            break;
        }
        case CheatId::INSTANT_DEATH: {
            if (p->is_ghost) {
                world.push_message(client_id, 0, "Ya estás muerto.");
                break;
            }
            p->hp = 0;
            p->is_ghost = true;
            p->meditating = false;
            world.update_occupied({p->pos_x, p->pos_y}, false);
            world.drop_player_loot(*p);
            world.push_message(client_id, 0, "CHEAT: has muerto instantáneamente.");
            break;
        }
        case CheatId::INSTANT_REVIVE: {
            if (!p->is_ghost) {
                world.push_message(client_id, 0, "No estás muerto.");
                break;
            }
            p->is_ghost   = false;
            p->meditating = false;
            p->hp = p->max_hp;
            p->mp = p->max_mp;
            world.update_occupied({p->pos_x, p->pos_y}, true);
            if (std::strlen(p->clan_name) > 0) world.clan_notify_login(client_id, true);
            world.push_message(client_id, 0, "CHEAT: has revivido instantáneamente.");
            break;
        }
    }
}
