#ifndef SMASH_REDIR_H_
#define SMASH_REDIR_H_

#include "Command.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

class RedirectionCommand : public Command
{
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line):Command(cmd_line){}
    virtual ~RedirectionCommand() {}
    void execute() override{

        SmallShell &shell = SmallShell::getInstance();
    string cmd_s = cmd_line;
    size_t re_sign_index ;
        if(((re_sign_index=cmd_s.find(">"))!=string::npos)&&(cmd_s.find(">>") == string::npos)){
            cout << " i am here in redirection " << endl;
            string left_command = cmd_s.substr(0,re_sign_index);
            _trim(left_command);
            cout << " the left command is " << left_command << endl;
            if (_isBackgroundComamnd(cmd_line)){
                left_command.push_back('&');
            }
            string file = cmd_s.substr(re_sign_index+ 1);
            _trim(file);
            cout << " the file is " << file << endl;
            char* filename=new char[strlen(file.c_str())+1];
            strcpy(filename,file.c_str());
            if(_isBackgroundComamnd(filename)){
                _removeBackgroundSign(filename);
            }
            int Fd1 = open(filename, O_TRUNC | O_RDWR | O_CREAT, 0644);
            if (Fd1 == -1) {
                perror("smash error: open failed");
                return ;
            }
            cout << " I am doing redirection " << endl;
            int back_fd = dup(1);
            SmallShell::getInstance().fd = dup(1);
            dup2(Fd1, 1);
            close(Fd1);
            Command* command = shell.CreateCommand(left_command.c_str());
            if(! command )
                return;
            command->execute();
            shell.jobs->removeFinishedJobs();
            fflush(stdout);
            dup2(back_fd, 1);
            close(back_fd);
            return;
        } else if ((re_sign_index = cmd_s.find(">>"))!=string::npos){
            string left_command = cmd_s.substr(0,re_sign_index);
            _trim(left_command);
            if (_isBackgroundComamnd(cmd_line)){
                left_command.push_back('&');
            }
            string file = cmd_s.substr( re_sign_index+ 2);
            _trim(file);
            char* filename=new char[strlen(file.c_str())+1];
            strcpy(filename,file.c_str());
            if(_isBackgroundComamnd(filename)){
                _removeBackgroundSign(filename);
            }
            int Fd1 = open(filename, O_APPEND | O_RDWR | O_CREAT, 0644);
            if (Fd1 == -1) {
                perror("smash error: open failed");
            }
            int back_fd = dup(1);
            SmallShell::getInstance().fd = dup(1);
            dup2(Fd1, 1);
            close(Fd1);
            Command *command = shell.CreateCommand(left_command.c_str());
             if (! command)
                 return;
             command->execute();
            fflush(stdout);
            dup2(back_fd, 1);
            close(back_fd);
            return ;
        }

    }
    //void prepare() override;
    //void cleanup() override;
};

#endif