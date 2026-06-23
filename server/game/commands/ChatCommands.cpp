#include <sstream>

#include "../Items.h"

#include "BankCommands.h"
#include "ChatCheatCommands.h"
#include "ClanCommands.h"
#include "Commands.h"
#include "DungeonCommands.h"
#include "MerchantCommands.h"

// Patrón aplicado: Command + tabla de dispatch (en vez de la cadena de 31
// if/else if que despachaba por texto de comando). Cada entrada de la
// tabla envuelve un handle_* o construye una de las clases auxiliares
// (BankCommands/MerchantCommands/ClanCommands/DungeonCommands/
// ChatCheatCommands) detrás de la misma firma uniforme, así agregar un
// comando nuevo es agregar una fila a la tabla. La tabla vive en una
// función-miembro privada (dispatch_table()) para poder llamar a los
// handle_* privados; es `static const` local, así que se construye una
// sola vez en todo el proceso aunque ChatCommand se instancie una vez por
// cada mensaje de chat recibido.
//
// SRP: ChatCommand ya no concentra banco, comerciante, clan, mazmorra y
// cheats de debug — cada responsabilidad vive en su propia clase auxiliar
// usada por composición (construida con el client_id dentro de la lambda
// que la necesita). Lo que queda acá es lo que no encaja en ninguna de
// esas clases: mensajería privada, meditar/resucitar/curar y tomar/tirar.
ChatCommand::ChatCommand(uint16_t c, std::string cmd): client_id(c), cmd(std::move(cmd)) {}

const ChatCommand::DispatchTable& ChatCommand::dispatch_table() {
    static const DispatchTable table = {
            {"/meditar", [](ChatCommand& s, World& w, const std::string&) { s.handle_meditar(w); }},
            {"/resucitar",
             [](ChatCommand& s, World& w, const std::string&) { s.handle_resucitar(w); }},
            {"/curar", [](ChatCommand& s, World& w, const std::string&) { s.handle_curar(w); }},
            {"/listar", [](ChatCommand& s, World& w, const std::string&) { s.handle_listar(w); }},
            {"/tomar", [](ChatCommand& s, World& w, const std::string&) { s.handle_tomar(w); }},
            {"/tirar",
             [](ChatCommand& s, World& w, const std::string& a) { s.handle_tirar(w, a); }},

            {"/depositar", [](ChatCommand& s, World& w,
                              const std::string& a) { BankCommands(s.client_id).deposit(w, a); }},
            {"/retirar", [](ChatCommand& s, World& w,
                            const std::string& a) { BankCommands(s.client_id).withdraw(w, a); }},

            {"/comprar", [](ChatCommand& s, World& w,
                            const std::string& a) { MerchantCommands(s.client_id).buy(w, a); }},
            {"/vender", [](ChatCommand& s, World& w,
                           const std::string& a) { MerchantCommands(s.client_id).sell(w, a); }},

            {"/fundar-clan", [](ChatCommand& s, World& w,
                                const std::string& a) { ClanCommands(s.client_id).found(w, a); }},
            {"/unirse", [](ChatCommand& s, World& w,
                           const std::string& a) { ClanCommands(s.client_id).join_request(w, a); }},
            {"/revisar-clan", [](ChatCommand& s, World& w,
                                 const std::string&) { ClanCommands(s.client_id).review(w); }},
            {"/clan-aceptar", [](ChatCommand& s, World& w,
                                 const std::string& a) { ClanCommands(s.client_id).accept(w, a); }},
            {"/clan-rechazar",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ClanCommands(s.client_id).reject(w, a);
             }},
            {"/clan-ban", [](ChatCommand& s, World& w,
                             const std::string& a) { ClanCommands(s.client_id).ban(w, a); }},
            {"/clan-kick", [](ChatCommand& s, World& w,
                              const std::string& a) { ClanCommands(s.client_id).kick(w, a); }},
            {"/dejar-clan", [](ChatCommand& s, World& w,
                               const std::string&) { ClanCommands(s.client_id).leave(w); }},

            {"/entrar-mazmorra", [](ChatCommand& s, World& w,
                                    const std::string&) { DungeonCommands(s.client_id).enter(w); }},
            {"/salir-mazmorra", [](ChatCommand& s, World& w,
                                   const std::string&) { DungeonCommands(s.client_id).leave(w); }},
            {"/info-mazmorra", [](ChatCommand& s, World& w,
                                  const std::string&) { DungeonCommands(s.client_id).info(w); }},

            {"/set-nivel",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).set_nivel(w, a);
             }},
            {"/set-vida",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).set_vida(w, a);
             }},
            {"/set-fuerza",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).set_fuerza(w, a);
             }},
            {"/set-agilidad",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).set_agilidad(w, a);
             }},
            {"/set-inteligencia",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).set_inteligencia(w, a);
             }},
            {"/set-constitucion",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).set_constitucion(w, a);
             }},
            {"/morir-instantaneo",
             [](ChatCommand& s, World& w, const std::string&) {
                 ChatCheatCommands(s.client_id).morir_instantaneo(w);
             }},
            {"/revivir-instantaneo",
             [](ChatCommand& s, World& w, const std::string&) {
                 ChatCheatCommands(s.client_id).revivir_instantaneo(w);
             }},
            {"/obtener-objeto",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).obtener_objeto(w, a);
             }},
            {"/set-oro",
             [](ChatCommand& s, World& w, const std::string& a) {
                 ChatCheatCommands(s.client_id).set_oro(w, a);
             }},
    };
    return table;
}

void ChatCommand::execute(World& world) {
    std::istringstream ss(cmd);
    std::string token;
    ss >> token;

    if (!token.empty() && token[0] == '@') {
        handle_private_msg(world, cmd);
        return;
    }

    std::string rest;
    std::getline(ss, rest);
    if (!rest.empty() && rest[0] == ' ')
        rest = rest.substr(1);

    const auto& table = dispatch_table();
    auto it = table.find(token);
    if (it == table.end()) {
        world.push_message(client_id, 0, "Comando desconocido: " + token);
        return;
    }
    it->second(*this, world, rest);
}

void ChatCommand::handle_meditar(World& world) { MeditateCommand(client_id).execute(world); }

void ChatCommand::handle_resucitar(World& world) {
    // El chequeo de cercanía al sacerdote (y cuál de los dos) vive en
    // ResurrectCommand::execute, compartido con la tecla R.
    ResurrectCommand(client_id).execute(world);
}

void ChatCommand::handle_curar(World& world) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p)
        return;

    if (!world.player_near_service_npc(client_id, NpcId::PRIEST)) {
        world.push_message(client_id, 0,
                           "Debes estar cerca del Sacerdote para ser curado.\n"
                           "Acércate y haz click en él primero.");
        return;
    }
    if (p->is_ghost) {
        world.push_message(client_id, 0, "Eres un fantasma. Usa /resucitar primero.");
        return;
    }
    p->hp = p->max_hp;
    p->mp = p->max_mp;
    world.push_message(client_id, 0, "El Sacerdote te curó completamente.");
}

// /listar significa cosas distintas según el NPC de servicio que esté
// cerca (cuenta de banco o catálogo de comerciante): es un coordinador
// entre dos clases auxiliares, no lógica de ninguna de las dos por
// separado, por eso queda en ChatCommand.
void ChatCommand::handle_listar(World& world) {
    if (BankCommands(client_id).try_list_account(world))
        return;
    if (MerchantCommands(client_id).try_list_catalog(world))
        return;
    world.push_message(client_id, 0,
                       "Debes estar cerca del Banquero o Comerciante para usar este comando.");
}

void ChatCommand::handle_tomar(World& world) { PickCommand(client_id).execute(world); }

void ChatCommand::handle_tirar(World& world, const std::string& args) {
    PlayerData* p = world.get_player_mutable(client_id);
    if (!p)
        return;

    try {
        int slot = std::stoi(args);
        if (slot >= 0 && slot < PlayerData::INVENTORY_SIZE) {
            DropCommand(client_id, static_cast<uint8_t>(slot)).execute(world);
            return;
        }
    } catch (...) {}

    for (int i = 0; i < PlayerData::INVENTORY_SIZE; ++i) {
        if (p->inventory[i] == 0)
            continue;
        if (Items::exists(static_cast<ItemId>(p->inventory[i]))) {
            if (Items::name_equals_ci(Items::get(static_cast<ItemId>(p->inventory[i])).name,
                                      args)) {
                DropCommand(client_id, static_cast<uint8_t>(i)).execute(world);
                return;
            }
        }
    }
    world.push_message(client_id, 0, "No encontraste ese objeto en tu inventario.");
}

void ChatCommand::handle_private_msg(World& world, const std::string& full_cmd) {
    std::istringstream ss(full_cmd);
    std::string target_token;
    ss >> target_token;
    if (target_token.size() < 2)
        return;
    std::string target_nick = target_token.substr(1);

    std::string msg;
    std::getline(ss, msg);
    if (!msg.empty() && msg[0] == ' ')
        msg = msg.substr(1);

    uint16_t target_id = world.find_player_by_name(target_nick);
    if (target_id == 0) {
        world.push_message(client_id, 0, "Jugador " + target_nick + " no encontrado.");
        return;
    }

    const PlayerData* sender = world.find_player(client_id);
    std::string sender_name = sender ? std::string(sender->username) : "?";

    world.push_message(target_id, 2, "[" + sender_name + " → ti]: " + msg);
    world.push_message(client_id, 2, "[tú → " + target_nick + "]: " + msg);
}
