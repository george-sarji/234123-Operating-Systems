#include "Shell.h"
#include "BuiltIn.h"
#include "Utility.h"
#include "External.h"


SmallShell::~SmallShell()
{
    // TODO: add your implementation
}

Command *SmallShell::createBuiltInCommand(vector<string> &args)
{
    if (args[0] == "quit")
    {
        if (args[1] == "kill")
        {
            jobs->killAllJobs();
            exit(0);
        }
        exit(0);
    }
    if (args[0] == "pwd")
    {
        return new GetCurrDirCommand(args[0].c_str());
    }
    if (args[0] == "showpid")
    {
        return new ShowPidCommand(args[0].c_str());
    }
    if (args[0] == "cd")
    {
        return new ChangeDirCommand(args[1].c_str(), args[1]);
    }
    if (args[0] == "kill")
    {
        return new KillCommand(args[0].c_str(), jobs);
    }
    if (args[0] == "fg")
    {
        return new ForegroundCommand(args[0].c_str(), jobs);
    }
    if (args[0] == "jobs")
    {
        return new JobsCommand(args[0].c_str(), jobs);
    }
    return nullptr;
}

Command *SmallShell::createExternalCommand(vector<string> &args)
{
    return nullptr;
}
Command *SmallShell::CreateCommand(const char *cmd_line)
{
    vector<string> args = analyseTheLine(cmd_line);
    const bool is_in = BuiltinTable.find(args[0]) != BuiltinTable.end();
    if (is_in)
    {
        Command *comm = createBuiltInCommand(args);
        return comm;
    }
    if (args[1] == "|" || args[1] == "|&")
    {
        Command *command = createPipeCommand(args);
        return command;
    }
    return new ExternalCommand(cmd_line);
}

void SmallShell::executeCommand(const char *cmd_line)
{
    // TODO: Add your implementation here

    Command *cmd = CreateCommand(cmd_line);
    if (!cmd)
        return;
    cmd->execute();
    delete cmd;
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

Command *SmallShell::createPipeCommand(vector<string> vector)
{
    return nullptr;
}
