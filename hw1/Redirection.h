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
    size_t re_sign1_index =cmd_s.find(">") ;
    size_t re_sign2_index = cmd_s.find(">>");

        if( re_sign1_index < re_sign2_index   && re_sign1_index != string::npos ){
            string left_command = cmd_s.substr(0,re_sign1_index);
            left_command = _trim(left_command);
            if (_isBackgroundComamnd(cmd_line)){
                left_command.push_back('&');
            }
            string file = cmd_s.substr(re_sign1_index+ 1);
            file = _trim(file);
//            cout <<"the file name is " << file<<endl;
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
            int back_fd = dup(1);
            SmallShell::getInstance().fd = dup(1);
            dup2(Fd1, 1);
            close(Fd1);
            Command* command = shell.CreateCommand(left_command.c_str());
            if(! command )
            {
                cerr << " ERROR " <<endl;
                return;
            }
            command->execute();
            shell.jobs->removeFinishedJobs();
            fflush(stdout);
            dup2(back_fd, 1);
            close(back_fd);
            return;
        }

        else if (((cmd_s.find(">>"))!=string::npos) ){
            string left_command = cmd_s.substr(0,re_sign2_index);
            left_command = _trim(left_command);
            if (_isBackgroundComamnd(cmd_line)){
                left_command.push_back('&');
            }
            string file = cmd_s.substr( re_sign2_index+ 2);
            file = _trim(file);
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