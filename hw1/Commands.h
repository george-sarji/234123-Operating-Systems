#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <list>
#include "time.h"
#include <set>

using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

enum JOB_TYPE {
	STOP,
	BACKGROUND
};
enum COMMAND_STATUS {
	ACTIVE,
	STOPPED,
	FINISH
};


class Command
{
public:
	COMMAND_STATUS status;
	int job_id;
	std::vector<string> arguments;
	// TODO: Add your data members

	Command(const char *cmd_line) {}
	virtual ~Command() {}
	virtual void execute() = 0;
	//virtual void prepare();
	//virtual void cleanup();
	// TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command
{
public:
	explicit BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}
	virtual ~BuiltInCommand() = default;
};
//
//class ExternalCommand : public Command {
//public:
//ExternalCommand(const char* cmd_line);
//virtual ~ExternalCommand() {}
//void execute() override;
//};
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
class JobsList
{
public:
	class JobEntry
	{

		/*
			Member descriptions
			job_id		Current job ID
			p_id		Current process ID
			background	Boolean if the process is background or not
			to_delete	Flag to check whether to delete process or not
			stopped		Activity status of process
			inserted 	Time (in seconds) when the job was first inserted
		*/
    public:
	    JobEntry(int Jop_id,int p_id,Command* command1,JOB_TYPE jobType):job_id(Jop_id),p_id(p_id),to_delete(false),
	    timestamp(),type(jobType),command(command1){
	        time(&timestamp);
	    }
		int job_id, p_id;
		bool to_delete;
		time_t timestamp;
		JOB_TYPE type;
		Command* command;
		bool operator==(JobEntry& jobEntry){
            return job_id== jobEntry.job_id && p_id==jobEntry.p_id;
		}

		bool operator!=(JobEntry& jobEntry){
            return ! this->operator==(jobEntry);
		}

		bool operator>(JobEntry jobEntry){
            return job_id > jobEntry.job_id;
		}

		bool operator<(JobEntry jobEntry){
            return ! this->operator>(jobEntry);
		}

		// TODO: Add your data members
	};

private:
	// TODO: Add your data members
	int next_id = 1;
	std::vector<JobEntry> jobs;
	std::vector<int> vacant_ids;

public:
	JobsList(): next_id(1), jobs(), vacant_ids() {}
	~JobsList() = default;
	void addJob(Command *cmd, bool isStopped = false);
	void printJobsList();
	void killAllJobs();
	void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
	void removeJobById(int jobId);
	JobEntry *getLastJob(int *lastJobId);
	JobEntry *getLastStoppedJob(int *jobId);
	// TODO: Add extra methods or modify exisitng ones as needed
};
//
//class JobsCommand : public BuiltInCommand {
// // TODO: Add your data members
// public:
//  JobsCommand(const char* cmd_line, JobsList* jobs);
//  virtual ~JobsCommand() {}
//  void execute() override;
//};
//
//class KillCommand : public BuiltInCommand {
// // TODO: Add your data members
// public:
//  KillCommand(const char* cmd_line, JobsList* jobs);
//  virtual ~KillCommand() {}
//  void execute() override;
//};
//
//class ForegroundCommand : public BuiltInCommand {
// // TODO: Add your data members
// public:
//  ForegroundCommand(const char* cmd_line, JobsList* jobs);
//  virtual ~ForegroundCommand() {}
//  void execute() override;
//};
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

class SmallShell
{

private:
	// TODO: Add your data members
	SmallShell() = default;
	std::vector<Command *> bk_jobs;
	std::vector<Command *> stopped_jobs;
	JobsList jobs;
	std::list<string> paths;
	std::string prompt = "smash";

public:
	Command *CreateCommand(const char *cmd_line);
	Command *createBuiltInCommand(vector<string> &args);
	SmallShell(SmallShell const &) = delete;	 // disable copy ctor
	void operator=(SmallShell const &) = delete; // disable = operator
	static SmallShell &getInstance()			 // make SmallShell singleton
	{
		static SmallShell instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}
	~SmallShell();
	void executeCommand(const char *cmd_line);
	// TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
