#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout<<"smash: got ctrl-Z"<<endl;
    SmallShell& shell = SmallShell::getInstance();
    pid_t pid = SmallShell::getInstance().curr_pid;
    if(pid == 0)
        return;
    SmallShell::getInstance().curr_pid = 0;
//    JobsList::JobEntry* job = shell.getJobs()->getJobByPid(pid);
//    if(job != nullptr)
//        job->stopIT();
    int res;

        res=kill(pid, SIGSTOP);

    if(res==0){

        shell.jobs->addJob(shell.curr_command,pid,true);
        cout << "smash: process " << pid << " was stopped" << endl;
    }
//    Command* cmd = shell.CreateCommand(shell.cmd_line,&flag,&(shell.pwd_current));
    // cout << "got here" <<  endl;
//    shell.jobsList->removeFinishedJobs();
    //cout << "got here" <<  endl;

}

void ctrlCHandler(int sig_num) {
    cout<<"smash: got ctrl-C"<<endl;
    pid_t pid = SmallShell::getInstance().curr_pid;
    SmallShell::getInstance().curr_pid = 0;
    if(pid == 0)
        return;

        kill(pid,SIGKILL);
    cout << "smash: process " << pid << " was killed" << endl;
    // TODO: Add your implementation
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

