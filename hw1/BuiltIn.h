#ifndef SMASH_BUILTIN_H_
#define SMASH_BUILTIN_H_

#include <set>
#include <string>
using namespace std;

#include "JobsList.h"
#include "Command.h"

extern set<string> BuiltinTable;

class BuiltInCommand : public Command
{
public:
    explicit BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}
    virtual ~BuiltInCommand() = default;
};

class ChangeDirCommand : public BuiltInCommand
{
    string path;

public:
    ChangeDirCommand(const char *cmd_line, string &plastPwd) : BuiltInCommand(cmd_line), path(plastPwd) {}
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
    explicit GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
    explicit ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class QuitCommand : public BuiltInCommand
{
    QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~QuitCommand() {}
    void execute() override;
};

class JobsCommand : public BuiltInCommand
{
public:
    JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {}
    virtual ~JobsCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand
{
    JobsList *jobsList;

public:
    KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobsList(jobs) {}
    virtual ~KillCommand() {}
    void execute() override;
};
//
class ForegroundCommand : public BuiltInCommand
{
    JobsList *Jobs;

public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), Jobs(jobs) {}
    virtual ~ForegroundCommand() = default;
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
public:
    BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~BackgroundCommand() {}
    void execute() override;
};
//
//class HeadCommand : public BuiltInCommand {
// public:
//  HeadCommand(const char* cmd_line);
//  virtual ~HeadCommand() {}
//  void execute() override;
//};

#endif