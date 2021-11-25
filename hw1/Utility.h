#ifndef SMASH_UTILITY_H_
#define SMASH_UTILITY_H_

#include <string>
#include <vector>
#include "BuiltIn.h"
#include "JobsList.h"

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using namespace std;

extern std::string WHITESPACE;

bool isNumber(const string &str);

string _ltrim(const std::string &s);

string _rtrim(const std::string &s);

string _trim(const std::string &s);

bool _isBackgroundComamnd(const char *cmd_line);

void _removeBackgroundSign(char *cmd_line);

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
vector<string> analyseTheLine(const char *cmd_line);

string getCommand(vector<string> args);

JobsList::JobEntry* resumeLastStoppedJob();

#endif