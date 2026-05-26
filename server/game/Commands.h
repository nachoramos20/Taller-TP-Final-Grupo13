#ifndef COMMANDS_H
#define COMMANDS_H

#include "Game.h"

class ServerCommand {
public:
    virtual void execute(Game &game)=0;
};

#endif // COMMANDS_H