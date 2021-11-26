#ifndef SMASH_REDIR_H_
#define SMASH_REDIR_H_

#include "Command.h"

class RedirectionCommand : public Command
{
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

#endif