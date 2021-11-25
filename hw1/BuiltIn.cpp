#include <unistd.h>
#include <errno.h>
#include <string>
#include <stdio.h>
#include <cstring>
#include <iostream>

#include "BuiltIn.h"
#include "Shell.h"
#include "Utility.h"
#include "JobsList.h"

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
    if (!args[2].empty())
    {
        // Too much arguments.
        cout << "smash error: cd: too many arguments" << endl;
        return;
    }
    string future_path, current_path = get_current_dir_name();
    if (args[1].compare("-") == 0)
    {
        if (smash.paths.empty())
        {
            cout << "smash error: cd: OLDPWD not set " << endl;
            return;
        }
        future_path = smash.paths.back();
        smash.paths.pop_back();
    }
    else
    {
        future_path = args[1];
    }
    // Now we have to change the directory.
    int result = chdir(future_path.c_str());
    if (result != 0)
    {
        // Send out an error.
        cout << strerror(errno) << endl;
    }
    else
    {
        // We have to push the old path into the path vector.
        smash.paths.push_back(current_path);
    }
}

void KillCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    vector<string> args = smash.curr_arguments;
    // We have to check the arguments.
    // Check if we have a second and a third argument.
    if (args[1].empty() || args[2].empty() || !args[3].empty())
    {
        cout << "smash error: kill: invalid arguments" << endl;
    }
    // We have to check the format of the first argument (the actual signal number)
    string signal_str = args[1];
    string signal_num = signal_str.substr(1, signal_str.length());
    string job_str = args[2];
    // If we have an invalid argument format, exit.
    if (signal_str[0] != '-' || !isNumber(signal_num) || !isNumber(job_str))
    {
        // Invalid format for argument. Exit.
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    // Check for the job ID in the jobs list.
    int signal = stoi(signal_num);
    JobsList::JobEntry *job = smash.jobs->getJobById(stoi(job_str));
    if (job == nullptr)
    {
        // Invalid job id. Exit.
        cout << "smash error: kill: job-id " << job_str << " does not exist" << endl;
        return;
    }
    // We can now send the signal to the process.
    int result = kill(job->p_id, stoi(signal_num));
    if (result != 0)
    {
        // We have an error.
        cout << strerror(errno) << endl;
        return;
    }
    // Worked. Send a message.
    cout << "signal number " << signal_num << " was sent to pid " << job->p_id << endl;
    // Update the job if it's a stopping signal.
    if (signal == SIGTSTP || signal == SIGINT || signal == SIGSTOP)
    {
        cout << "Stopped with " << signal_num << endl;
        // Change the job to stopped.
        job->stop();
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

void BackgroundCommand::execute()
{
    // Get the shell instance
    SmallShell &shell = SmallShell::getInstance();
    // Get the arguments.
    vector<string> arguments = shell.curr_arguments;
    // Check if we have arguments in the command itself.
    if (arguments.size() == 1 || arguments[1].empty())
    {
        // We have no arguments, get the last stopped job.
        JobsList::JobEntry *job = resumeLastStoppedJob();
        // Did we receive a job?
        if (job == nullptr)
        {
            // No stopped jobs. Give an error.
            cout << "smash error: bg: there is no stopped jobs to resume" << endl;
        }
        return;
    }
    else
    {
        // We have an argument. Get the ID.
        int job_id = stoi(arguments[1]);
        // Get the job with the given ID.
        JobsList::JobEntry *job = shell.jobs->getJobById(job_id);
        // Check if we have a job with the given ID.
        if (job == nullptr)
        {
            // No job with the ID. Print an error.
            cout << "smash error: bg: job-id " << job_id << " does not exist" << endl;
        }
        // Check if the job is stopped.
        else if (!job->stopped)
        {
            // Give out an error, job is still running.
            cout << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
        }
        else
        {
            // Job is valid and paused. Resume it.
            job->_continue_();
        }
    }
}

void QuitCommand::execute()
{
    SmallShell &shell = SmallShell::getInstance();
    vector<string> arguments = shell.curr_arguments;
    // Check if we have kill provided.
    if (!arguments[1].empty() && arguments[1].compare("kill") == 0)
    {
        // We have received a kill flag. Go over the jobs and send SIGKILL.
        vector<JobsList::JobEntry> jobs = shell.jobs->jobs;
        // Get the count.
        cout << "smash: sending SIGKILL signal to " << shell.jobs->size << " jobs:" << endl;
        sort(jobs.begin(), jobs.end());
        for (auto it = jobs.begin(); it != jobs.end(); ++it)
        {
            // Send the sigkills and print the command itself.
            cout << it->p_id << ": " << it->command << endl;
            kill(it->p_id, SIGKILL);
        }
    }
    // We can exit the shell completely.
    exit(0);
}