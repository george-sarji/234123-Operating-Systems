#include "Utility.h"
#include "Shell.h"
#include "Command.h"
#include <vector>

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

Command* getAndRemoveLastStoppedCommand() {
    // Get the shell instance.
    SmallShell& shell = SmallShell::getInstance();
    vector<Command*> stopped = shell.stopped_jobs;
    if(stopped.size() == 0) return nullptr;
    // Iterate and find the one with the maximal job ID.
    vector<Command*>::iterator max_it = stopped.begin();
    Command* max = stopped.front();
    for(auto it = stopped.begin(); it != stopped.end(); ++it) 
    {
        // Check if we have a higher job ID.
        if((*it)->job_id > max->job_id) 
        {
            // New iterator assignment.
            max_it = it;
            max = *it;
        }
    }
    // Erase the item.
    stopped.erase(max_it);
    return max;
}