#include "Commands.h"
#include "../Npc.h"
#include <cstdlib>

static int manhattan(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

NpcInteractCommand::NpcInteractCommand(uint16_t c, uint16_t n)
    : client_id(c), npc_id(n) {}

void NpcInteractCommand::execute(World& world) {
    NpcData* npc = world.find_npc(npc_id);
    if (!npc) {
        world.push_message(client_id, 0, "NPC no encontrado.");
        return;
    }

    const NpcTemplate& tpl = Npcs::tpl(npc->type);
    const PlayerData*  p   = world.find_player(client_id);
    if (!p) return;

    if (!tpl.is_service) {
        // Es un enemigo, ignorar interacción (el cliente debería atacar, no interactuar)
        world.push_message(client_id, 0, tpl.name + " no quiere hablar contigo.");
        return;
    }

    // Verificar proximidad (máximo 2 tiles de distancia Manhattan)
    int dist = manhattan(p->pos_x, p->pos_y, npc->pos_x, npc->pos_y);
    if (dist > 2) {
        world.push_message(client_id, 0,
            "Estás demasiado lejos de " + tpl.name + ". Acércate.");
        return;
    }

    // Registrar el NPC seleccionado en el mundo para que los comandos lo validen
    world.set_selected_npc(client_id, npc->type);

    switch (npc->type) {
        case NpcId::MERCHANT:
            world.push_message(client_id, 0,
                "=== Comerciante ===\n"
                "Usa /listar para ver los articulos disponibles.\n"
                "Usa /comprar <nombre> para comprar.\n"
                "Usa /vender <nombre> para vender.");
            break;
        case NpcId::BANKER:
            world.push_message(client_id, 0,
                "=== Banquero ===\n"
                "Usa /listar para ver tu cuenta.\n"
                "Usa /depositar oro <cantidad> o /depositar <objeto>.\n"
                "Usa /retirar oro <cantidad> o /retirar <objeto>.");
            break;
        case NpcId::PRIEST:
            world.push_message(client_id, 0,
                "=== Sacerdote ===\n"
                "Usa /curar para recuperar toda tu vida y mana.\n"
                "Usa /resucitar si eres un fantasma.");
            break;
        default:
            break;
    }
}
