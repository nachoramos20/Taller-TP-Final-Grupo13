#ifndef COMMANDS_H
#define COMMANDS_H

#include "Game.h"

class ServerCommand {
public:
    virtual void execute(Game &game)=0;
    virtual ~ServerCommand() = default;
};

class MoveCommand : public ServerCommand {
public:
    MoveCommand(uint16_t client_id, uint16_t new_x, uint16_t new_y);
    void execute(Game &game) override;

private:
    uint16_t client_id;
    uint16_t new_x;
    uint16_t new_y;
};

class LoginCommand : public ServerCommand {
public:
    LoginCommand(uint16_t client_id, const std::string& username);
    void execute(Game &game) override;
private:    
    uint16_t client_id;
    std::string username;
};


#endif // COMMANDS_H