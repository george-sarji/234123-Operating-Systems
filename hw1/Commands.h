#ifndef SMASH_COMMANDS_H_
#define SMASH_COMMANDS_H_

#include <vector>
#include <string>
#include <list>
#include "time.h"
#include <set>
#include <csignal>

#include "JobsList.h"
#include "Command.h"
#include "BuiltIn.h"

using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)


//
//class PipeCommand : public Command {
//    // TODO: Add your data members
//public:
//    PipeCommand(const char* cmd_line);
//    virtual ~PipeCommand() {}
//    void execute() override;
//};
//
//class RedirectionCommand : public Command {
// // TODO: Add your data members
// public:
//  explicit RedirectionCommand(const char* cmd_line);
//  virtual ~RedirectionCommand() {}
//  void execute() override;
//  //void prepare() override;
//  //void cleanup() override;
//};
//



#endif //SMASH_COMMAND_H_
