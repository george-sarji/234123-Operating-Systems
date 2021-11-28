#ifndef SMASH_PIPE_H_
#define SMASH_PIPE_H_

#include "Command.h"

class PipeCommand : public Command
{
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line):Command(cmd_line){
    }
    virtual ~PipeCommand() {}
    void execute() override{

    }
};



#endif