#ifndef SMASH_EXTERNAL_H_
#define SMASH_EXTERNAL_H_

#include "Command.h"

class ExternalCommand : public Command
{
public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line) {}
    virtual ~ExternalCommand() {}
    void execute() override;
};

#endif