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

using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

extern set<string> BuiltinTable;

vector<string> analyseTheLine(const char *cmd_line);

class BuiltInCommand : public Command
{
public:
	explicit BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}
	virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command
{
public:
	ExternalCommand(const char *cmd_line) : Command(cmd_line) {}
	virtual ~ExternalCommand() {}
	void execute() override;
};
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
class ChangeDirCommand : public BuiltInCommand
{
	string path;

public:
	// TODO: Add your data members public:
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
	explicit ShowPidCommand(const char *cmd_line);
	virtual ~ShowPidCommand() {}
	void execute() override;
};

//class JobsList;
//class QuitCommand : public BuiltInCommand {
//// TODO: Add your data members public:
//  QuitCommand(const char* cmd_line, JobsList* jobs);
//  virtual ~QuitCommand() {}
//  void execute() override;
//};
//
//
//
//

class JobsCommand : public BuiltInCommand
{

	// TODO: Add your data members
public:
	JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) {}
	virtual ~JobsCommand() {}
	void execute() override;
};

class KillCommand : public BuiltInCommand
{
	JobsList *jobsList;
	// TODO: Add your data members
public:
	KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobsList(jobs) {}
	virtual ~KillCommand() {}
	void execute() override;
};
//
class ForegroundCommand : public BuiltInCommand
{
	JobsList *Jobs;
	// TODO: Add your data members
public:
	ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), Jobs(jobs) {}
	virtual ~ForegroundCommand() = default;
	void execute() override;
};
//
//class BackgroundCommand : public BuiltInCommand {
// // TODO: Add your data members
// public:
//  BackgroundCommand(const char* cmd_line, JobsList* jobs);
//  virtual ~BackgroundCommand() {}
//  void execute() override;
//};
//
//class HeadCommand : public BuiltInCommand {
// public:
//  HeadCommand(const char* cmd_line);
//  virtual ~HeadCommand() {}
//  void execute() override;
//};

#endif //SMASH_COMMAND_H_
