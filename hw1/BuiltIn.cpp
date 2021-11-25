#include <unistd.h>
#include <errno.h>
#include <string>
#include <stdio.h>
#include <cstring>

#include "BuiltIn.h"
#include "Shell.h"
#include "Utility.h"

set<string> BuiltinTable{"cd", "chprompt", "showpid", "pwd", "jobs", "kill", "fg", "bg", "quit"};

void ShowPidCommand::execute()
{
    int pid = getpid();
    cout << "smash pid is " << pid << endl;
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

void JobsCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    smash.jobs->printJobsList();
}