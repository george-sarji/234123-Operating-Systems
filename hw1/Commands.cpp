#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <utility>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <cerrno>
#include <algorithm>
#include <csignal>

#include "JobsList.h"
#include "Shell.h"

using namespace std;

#if 0
#define FUNC_ENTRY() \
    cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
    cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

set<string> BuiltinTable{"cd", "chprompt", "showpid", "pwd", "jobs", "kill", "fg", "bg", "quit"};

std::string WHITESPACE(" \t\f\v\n\r");

bool isNumber(const string &str)
{
    for (char const &c : str)
    {
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}

string _ltrim(const std::string &s)
{
    std::string WHITESPACE(" \t\f\v\n\r");
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
    std::string WHITESPACE(" \t\f\v\n\r");
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
    return _rtrim(_ltrim(s));
}

//int _parseCommandLine(const char* cmd_line, char** args) {
//  FUNC_ENTRY()
//  int i = 0;
//  std::istringstream iss(_trim(string(cmd_line)).c_str());
//  for(std::string s; iss >> s; ) {
//    args[i] = (char*)malloc(s.length()+1);
//    memset(args[i], 0, s.length()+1);
//    strcpy(args[i], s.c_str());
//    args[++i] = NULL;
//  }
//  return i;
//
//  FUNC_EXIT()
//}

bool _isBackgroundComamnd(const char *cmd_line)
{
    const string str(cmd_line);
    std::string WHITESPACE(" \t\f\v\n\r");
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
    const string str(cmd_line);
    // find last character other than spaces
    std::string WHITESPACE(" \t\f\v\n\r");
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos)
    {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&')
    {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
vector<string> analyseTheLine(const char *cmd_line)
{

    string cmd_s = cmd_line;
    vector<string> args(20, "");
    cmd_s = _trim(string(cmd_s));
    string first_word = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (_isBackgroundComamnd(first_word.c_str()))
    {
        string cmd_s1 = cmd_s;
        cmd_s1.erase(cmd_s1.find_first_of("&"), 1);
        cmd_s1 = _trim(string(cmd_s1));
        first_word = cmd_s1.substr(0, cmd_s1.find_first_of(" \n"));
        if (BuiltinTable.count(first_word))
        {
            cmd_s = cmd_s1;
        }
    }
    int i = 0;
    for (auto index = args.begin(); index != args.end(); index++)
    {
        string arg = cmd_s.substr(0, cmd_s.find_first_of(WHITESPACE));
        if (arg.empty())
        {

            break;
        }
        args[i] = arg;
        cmd_s = cmd_s.substr(arg.size());
        cmd_s = _trim(string(cmd_s));
        i++;
    }
    return args;
}

void exeuteFgCommand(const std::vector<string> &args)
{
}

void ShowPidCommand::execute()
{
    int pid = getpid();
    cout << "smash pid is " << pid << endl;
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void GetCurrDirCommand::execute()
{

    char path[100];
    getcwd(path, 100);
    string str(path);
    cout << str << endl;
}

void ChangeDirCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    vector<string> args = smash.curr_arguments;
    if (args[1].empty())
        return;
    else if (args[1].compare("-") == 0)
    {
        if (smash.paths.empty())
        {
            cout << "smash error: cd: OLDPWD not set " << endl;
            return;
        }
        if (!args[2].empty())
        {
            cout << "smash error: cd: too many arguments" << endl;
            return;
        }
        string path1 = smash.paths.back();
        smash.paths.pop_back();
        int res = chdir(path1.c_str());
        if (res < 0)
        {
            cout << strerror(errno) << endl;
        }
        return;
    }
    else if (args[1].compare("..") == 0)
    {
        if (smash.paths.empty())
        {
            return;
        }
        string str = smash.paths.back();
    }

    if (!args[2].empty())
    {
        cout << "smash error: cd: too many arguments" << endl;
    }
    else
    {
        string path1 = smash.paths.back();
        int res = chdir(path1.c_str());
        if (res < 0)
        {
            cout << strerror(errno) << endl;
        }
        return;
    }
}

void KillCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    vector<string> args = smash.curr_arguments;

    if (!args[3].empty() || args[1] != "-9" || args[2].empty())
    {
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    string job_id = args[2];
    if (isNumber(args[2]))
    {
        if (smash.jobs->getJobById(stoi(job_id)))
        {
            cout << "i am here  in kill " << endl;
            cout << "the proc id is " << jobsList->getJobById(stoi(job_id))->p_id << endl;
            kill(jobsList->getJobById(stoi(job_id))->p_id, 9);
            jobsList->removeJobById(stoi(job_id));
            return;
        }
        else
        {
            if (isNumber(args[2]))
            {
                cout << "smash error: kill: job-id " << args[2] << " does not exist" << endl;
            }
            else
                cout << "smash error: kill: invalid arguments" << endl;
        }
    }
    else
    {
        cout << "smash error: kill: invalid arguments" << endl;
    }
}

void ForegroundCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    vector<string> args = smash.curr_arguments;

    if (args[1].empty())
    {
        if (smash.jobs->empty())
        {
            cout << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        else
        {
            JobsList::JobEntry *jobEntry = smash.jobs->getLastJob();
            if (jobEntry)
            {
                if (jobEntry->stopped)
                {
                    jobEntry->_continue_();
                }

                int pid = jobEntry->p_id;
                //            int status;
                waitpid(pid, NULL, WUNTRACED);
                //            cout <<" the res is " << status<<endl;
                return;
            }
        }
    }
    else if (args[2].empty() && isNumber(args[1]))
    {
        JobsList::JobEntry *job = smash.jobs->getJobById(stoi(args[1]));
        if (job)
        {
            if (job->stopped)
            {
                job->_continue_();
            }
            pid_t pid = job->p_id;
            waitpid(pid, NULL, WUNTRACED);
            //            cout <<" the res is " << st<<endl;
            return;
        }
        else
        {
            if (isNumber(args[1]))
            {
                cout << "smash error: fg: job-id " << args[1] << " does not exist" << endl;
                return;
            }
            else
                cout << "smash error: fg: invalid arguments" << endl;
            return;
        }
    }
    cout << "smash error: fg: invalid arguments" << endl;
}

string getCommand(vector<string> args)
{
    string cmd;
    for (auto &arg : args)
    {
        cmd += arg;
        cmd += " ";
    }
    cmd = _trim(cmd);
    return cmd;
}
void ExternalCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    vector<string> args = smash.curr_arguments;
    string cmd_line1 = getCommand(args);

    int p_id = fork();
    if (p_id == 0)
    {

        setpgrp();

        int len;

        len = strlen(cmd_line) + 1;

        char *cmd_line_t_t = new char[len];

        strcpy(cmd_line_t_t, cmd_line);

        if (_isBackgroundComamnd(cmd_line))
        {
            _removeBackgroundSign(cmd_line_t_t);
        }

        char s1[10] = "/bin/bash";

        char s2[3] = "-c";

        char *const exe_args[] = {s1, s2, cmd_line_t_t, nullptr};
        execv("/bin/bash", exe_args);
        printf("didn't work\n");
    }
    else if (_isBackgroundComamnd(cmd_line))
        smash.jobs->addJob(cmd_line1, p_id);
    else
    {
        smash.curr_pid = p_id;
        smash.curr_command = cmd_line1;
        waitpid(p_id, NULL, WUNTRACED);
        smash.curr_pid = 0;
        smash.curr_command = "";
    }
}

void JobsCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    smash.jobs->printJobsList();
}