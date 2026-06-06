#include "Commands.h"

NpcInteractCommand::NpcInteractCommand(uint16_t c, uint16_t n)
    : client_id(c), npc_id(n) {}

void NpcInteractCommand::execute(World& world) {
    NpcData* npc = world.find_npc(npc_id);
    if (!npc) {
        world.push_message(client_id, 0, "NPC no encontrado.");
        return;
    }
    const NpcTemplate& tpl = Npcs::tpl(npc->type);
    world.push_message(client_id, 0,
        "Seleccionaste a " + tpl.name + ". Usa /curar, /comprar, /depositar, etc.");
}
