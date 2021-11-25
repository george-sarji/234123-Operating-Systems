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


void exeuteFgCommand(const std::vector<string> &args)
{
}


