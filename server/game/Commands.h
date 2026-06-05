#ifndef COMMANDS_H
#define COMMANDS_H

#include "World.h"
#include "PlayerData.h"
#include "../../common/protocol/protocol.h"
#include <string>

class ServerCommand {
public:
    virtual void execute(World& world) = 0;
    virtual ~ServerCommand() = default;
};

class MoveCommand : public ServerCommand {
public:
    MoveCommand(uint16_t client_id, uint16_t new_x, uint16_t new_y);
    void execute(World& world) override;
private:
    uint16_t client_id, new_x, new_y;
};

class LoginCommand : public ServerCommand {
public:
    LoginCommand(uint16_t client_id, const std::string& username);
    LoginCommand(PlayerData player_data);
    void execute(World &world) override;
    const char* get_username() const;
private:    
    PlayerData player_data;
};

class AttackCommand : public ServerCommand {
public:
    AttackCommand(uint16_t client_id, uint16_t target_id);
    void execute(World& world) override;
private:
    uint16_t client_id, target_id;
};

class EquipCommand : public ServerCommand {
public:
    EquipCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t inv_slot;
};

class UnequipCommand : public ServerCommand {
public:
    UnequipCommand(uint16_t client_id, EquipSlot slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    EquipSlot slot;
};

class DropCommand : public ServerCommand {
public:
    DropCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t inv_slot;
};

class PickCommand : public ServerCommand {
public:
    explicit PickCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class UseItemCommand : public ServerCommand {
public:
    UseItemCommand(uint16_t client_id, uint8_t inv_slot);
    void execute(World& world) override;
private:
    uint16_t client_id;
    uint8_t inv_slot;
};

class MeditateCommand : public ServerCommand {
public:
    explicit MeditateCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class ResurrectCommand : public ServerCommand {
public:
    explicit ResurrectCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class LogoutCommand : public ServerCommand {
public:
    explicit LogoutCommand(uint16_t client_id);
    void execute(World& world) override;
private:
    uint16_t client_id;
};

class NpcInteractCommand : public ServerCommand {
public:
    NpcInteractCommand(uint16_t client_id, uint16_t npc_id);
    void execute(World& world) override;
private:
    uint16_t client_id, npc_id;
};

class ChatCommand : public ServerCommand {
public:
    ChatCommand(uint16_t client_id, std::string cmd);
    void execute(World& world) override;
private:
    uint16_t client_id;
    std::string cmd;
};

#endif