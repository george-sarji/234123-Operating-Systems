#ifndef SMASH_PIPE_H_
#define SMASH_PIPE_H_

#include <unistd.h>
#include "Command.h"
#include "Shell.h"

#define READ 0
#define WRITE 1

class PipeCommand : public Command
{
    // TODO: Add your data members
    bool error_redirection;
    std::string left_operand, right_operand;

public:
    explicit PipeCommand(const char *cmd_line) : Command(cmd_line)
    {
        // We need to split the command according to the pipe symbol.
        size_t index = 0;
        std::string command = cmd_line;
        index = command.find("|&");
        error_redirection = index != string::npos;
        if (index == string::npos)
        {
            index = command.find("|");
            // Split according to index.
            left_operand = command.substr(0, index );
            right_operand = command.substr(index + 1);
        }
        else
        {
            left_operand = command.substr(0, index );
            right_operand = command.substr(index + 2);
        }
        left_operand = _trim(left_operand);
        right_operand = _trim(right_operand);
        // Now we need to split according to the index.
    }


    virtual ~PipeCommand() {}


    void execute() override
    {

        SmallShell &shell = SmallShell::getInstance();
        int is_backg = _isBackgroundComamnd(cmd_line);
        int Pipes[2];
        pid_t firstChild = -1;
        pid_t secondChild = -1;
// we have to produce a pipe which redirects command1 stdout to its write channel and command2 stdin to its read channel

        if (  ! error_redirection ){
            int output = dup(0);
            int output2 = dup(1);
            if(is_backg){
                left_operand+="&";
            }
            if(is_backg){
                right_operand+="&";
            }
            pipe(Pipes);
            if(fork()==0){
                setpgrp();
//                pid_t tester=getppid();
//                pid_t pidSonOne=getpid();
                firstChild = getpid();
                dup2(Pipes[WRITE], 1);
                close(Pipes[WRITE]);
                close(Pipes[READ]);
                shell.curr_command = left_operand;
                shell.curr_arguments = analyseTheLine(left_operand.c_str());
                Command *Command = shell.CreateCommand(left_operand.c_str());
                Command->execute();
                dup2(output2, 1);
                close(output2);
                exit(0);
            }
            dup2(output2, 1);
            if(fork()==0)
            {
                setpgrp();
                secondChild = getpid();
//                pid_t tester2=getppid();
                dup2(Pipes[READ], 0);
                close(Pipes[WRITE]);
                close(Pipes[READ]);
                shell.curr_command = right_operand;
                shell.curr_arguments = analyseTheLine(right_operand.c_str());
                Command *Command = shell.CreateCommand(right_operand.c_str());
                Command->execute();
                dup2(output, 0);
                close(output);
                exit(0);
            }
            close(Pipes[READ]);
            close(Pipes[WRITE]);
            if (waitpid(firstChild,NULL,0) == -1){
                perror("smash error: waitpid failed");
            }
            if (waitpid(secondChild,NULL,0) == -1){
                perror("smash error: waitpid failed");
            }
            return;
        }

// we have to produce a pipe
//which redirects command1 stderr to the pipe’s write channel and command2 stdin
//to the pipe’s read channel.
        else {
            int output = dup(0);
            int output3 = dup(2);
            if(is_backg){
                left_operand="&";
            }
            if(is_backg){
                right_operand+="&";
            }
            pipe(Pipes);
            if(fork()==0) {
                setpgrp();
                firstChild = getpid();
                dup2(Pipes[WRITE], 2);
                close(Pipes[WRITE]);
                close(Pipes[READ]);
                shell.curr_command = left_operand;
                shell.curr_arguments = analyseTheLine(left_operand.c_str());
                Command *Command = shell.CreateCommand(left_operand.c_str());
                Command->execute();
                dup2(output3, 2);
                close(output3);
                exit(0);
            }
            if(fork()==0)
            {
                setpgrp();
                secondChild = getpid();
                dup2(Pipes[READ], 0);
                close(Pipes[WRITE]);
                close(Pipes[READ]);
                shell.curr_command = right_operand;
                shell.curr_arguments = analyseTheLine(right_operand.c_str());
                Command* Command2=shell.CreateCommand(right_operand.c_str());
               Command2->execute();
                dup2(output, 0);
                close(output);
                exit(0);
            }
            close(Pipes[READ]);
            close(Pipes[WRITE]);
            if (waitpid(firstChild,NULL,0) == -1){
                perror("smash error: waitpid failed");
            }
            if (waitpid(secondChild,NULL,0) == -1){
                perror("smash error: waitpid failed");
            }
            return;
        }
//        // We need to redirect the output appropriately.
//        // Create the file descriptors.
//        int fd[2];
//        // We can open the pipe now.
//        pipe(fd);
//        // We need to fork.
//        int pid = fork();
//        if (pid == 0)
//        {
//            // We are in the child.
//            // Close the write channel.
//            dup2(fd[0], 0);
//            close(fd[WRITE]);
//            // Create the command and execute (right operand)
//            Command *command = shell.CreateCommand(right_operand.c_str());
//            // Check if external command.
//            command->execute();
//
//            close(fd[READ]);
//            exit(0);
//        }
//        else
//        {
//            // We are in the parent. We can close the read.
//            // We have to execute left operand.
//            int status = 0;
//            close(fd[READ]);
//            if (error_redirection)
//            {
//                dup2(fd[1], STDERR_FILENO);
//            }
//            else
//            {
//                dup2(fd[1], STDOUT_FILENO);
//            }
//            Command *command = shell.CreateCommand(left_operand.c_str());
//            // Check if command is an external command.
//            command->execute();
//            waitpid(pid, nullptr, WNOHANG);
//            close(fd[WRITE]);
//        }
    }
};

#endif
