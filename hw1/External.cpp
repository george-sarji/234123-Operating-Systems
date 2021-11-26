#include "External.h"
#include "Shell.h"
#include "Utility.h"
#include <vector>
#include <unistd.h>
#include <string.h>

using namespace std;

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
