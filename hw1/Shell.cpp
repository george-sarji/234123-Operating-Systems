#include "Shell.h"
#include "BuiltIn.h"
#include "Utility.h"
#include "External.h"
#include "Redirection.h"
#include "Pipe.h"


SmallShell::~SmallShell()
{
    // TODO: add your implementation
}

Command *SmallShell::createBuiltInCommand(vector<string> &args)
{
    if (args[0] == "quit")
    {
        return new QuitCommand(args[0].c_str());
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
    if (args[0] == "bg")
    {
        return new BackgroundCommand(args[0].c_str());
    }
    if (args[0] == "head"){
        return new HeadCommand(args[0].c_str());
    }
    return nullptr;
}


Command *SmallShell::CreateCommand(const char *cmd_line)
{
    vector<string> args = analyseTheLine(cmd_line);
    string cmd_s = cmd_line;
    const bool is_in = BuiltinTable.find(args[0]) != BuiltinTable.end();
    bool is_re_command = is_redirection_command(cmd_s , args);
    bool isBackground = _isBackgroundComamnd(cmd_line);
    if (is_in && !is_re_command && !isBackground)
    {
        Command *comm = createBuiltInCommand(args);
        return comm;
    }

    if ((cmd_s.find("|") != string::npos) || (cmd_s.find("|&") != string::npos)) {
        return new PipeCommand(cmd_line);
    }
    if ( is_re_command)
    {
         return new RedirectionCommand(cmd_line);
    }

    return new ExternalCommand(cmd_line);
}

void SmallShell::executeCommand(const char *cmd_line)
{
    // TODO: Add your implementation here

    Command *cmd = CreateCommand(cmd_line);
    if (!cmd)
        return;
    jobs->removeFinishedJobs();
    cmd->execute();
    delete cmd;
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

Command *SmallShell::createPipeCommand(vector<string> vector)
{
    return nullptr;
}

bool SmallShell::is_redirection_command(const string& cmd_s , vector<string> args) {
    if (std::count(args.begin(),args.end(),">") ||  std::count(args.begin(),args.end(),">>")){
        return  (cmd_s.find("|") == string::npos) ;
    }
    return false;
}
