#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include <vector>

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    int flag = 4;
    while(flag) {
        std::cout << smash.getPrompt()<<">";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        std::vector<std::string> args = analyseTheLine(cmd_line.c_str());
        smash.curr_arguments = args;
        if(args[0] == "chprompt"){
            if (args[1].empty()) {
                smash.changPrompt("smash");
            } else{
                smash.changPrompt(args[1]);
            }
        }
        smash.executeCommand(cmd_line.c_str());
        flag--;
    }
    return 0;
}