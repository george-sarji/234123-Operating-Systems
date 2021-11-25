#ifndef SMASH_SHELL_H_
#define SMASH_SHELL_H_

#include <vector>
#include <string>
#include <set>
#include <list>

using namespace std;

#include "Command.h"
#include "JobsList.h"


class SmallShell
{
public:
    // TODO: Add your data members
    SmallShell() : bk_jobs(), stopped_jobs(), paths(), curr_pid(0), curr_command()
    {
        this->jobs = new JobsList();
    }
    std::vector<Command *> bk_jobs;
    std::vector<Command *> stopped_jobs;
    JobsList *jobs;
    std::list<string> paths;
    std::string Prompt = "smash";
    int curr_pid;
    std::string curr_command;

    Command *CreateCommand(const char *cmd_line);
    Command *createBuiltInCommand(vector<string> &args);
    Command *createExternalCommand(vector<string> &args);
    SmallShell(SmallShell const &) = delete;     // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance()             // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char *cmd_line);
    std::string getPrompt()
    {
        return Prompt;
    }
    void changPrompt(const std::string &prompt)
    {
        Prompt = prompt;
    }
    vector<string> curr_arguments;
    // TODO: add extra methods as needed
    Command *createPipeCommand(vector<string> vector);
};

#endif