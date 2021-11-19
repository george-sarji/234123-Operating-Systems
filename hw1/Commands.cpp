#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <cerrno>
#include <algorithm>
#include <csignal>

using namespace std;

#if 0
#define FUNC_ENTRY() \
	cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
	cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

std::string WHITESPACE(" \t\f\v\n\r");
set<string> BuiltinTable{"cd", "chprompt", "showpid", "pwd", "jobs", "kill", "fg", "bg", "quit"};

bool isNumber(const string& str)
{
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
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

// TODO: Add your implementation for classes in Commands.h

//SmallShell::SmallShell() {
//// TODO: add your implementation
//}
//
SmallShell::~SmallShell()
{
	// TODO: add your implementation
}

//Command *SmallShell::createBuiltInCommand(vector<string> &args) {
//    return nullptr;
//}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
vector<string> analyseTheLine(const char *cmd_line)
{
	string cmd_s = cmd_line;
	vector<string> args(20, "");
	cmd_s = _trim(string(cmd_s));
	int i = 0;
	for (vector<string>::iterator index = args.begin(); index != args.end(); index++)
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

void exeuteFgCommand(const std::vector<string> & args) {

}

Command *SmallShell::createBuiltInCommand(vector<string> &args)
{
	if (args[0] == "pwd")
	{
		//        if(!args[1].empty()) return nullptr;
		return new GetCurrDirCommand(args[0].c_str());
	}
	if (args[0] == "showpid")
	{
		//        if(!args[1].empty()) {
		//            return nullptr;
		//        }
		return new ShowPidCommand(args[0].c_str());
	}
	if (args[0] == "cd")
	{
		if (args[1].empty())
			return nullptr;
		else if (args[1].compare("-") == 0)
		{
			if (paths.empty())
			{
				cout << "smash error: cd: OLDPWD not set " << endl;
				return nullptr;
			}
			if (!args[2].empty())
			{
				cout << "smash error: cd: too many arguments" << endl;
				return nullptr;
			}
			string path = paths.back();
			paths.pop_back();
			cout << args[1].c_str() << endl;
			return new ChangeDirCommand(args[1].c_str(), args[1]);
		}
		else if (args[1].compare("..") == 0)
		{
			if (paths.empty())
			{
				return nullptr;
			}
			string str = paths.back();
		}

		if (!args[2].empty())
		{
			cout << "smash error: cd: too many arguments" << endl;
		}
		else
		{
			string path = paths.back();
			return new ChangeDirCommand(args[0].c_str(), args[1]);
		}

	}
	if (args[0].compare("kill")==0){
        if(! args[3].empty() || args[1]!= "-9" || args[2].empty()){
            cout <<"smash error: kill: invalid arguments"<<endl;
            return nullptr;
        }
        string job_id = args[2];
        if(jobs.getJobById(stoi(job_id))){
            return new KillCommand(args[0].c_str(),&jobs,stoi(job_id));
        }
        else{
            cout <<"smash error: kill: job-id " << stoi((job_id))<<" does not exist" <<endl;
        }

    }
	if (args[0].compare("fg") == 0){
	    if(args[1].empty()){
	        if (jobs.empty()){
	            cout << "smash error: fg: jobs list is empty"<<endl;
	        }
	        else{
//                JobsList::JobEntry * cmd = jobs.getJobById(stoi(args[1]));
//	            exeuteFgCommand(cmd->command->arguments);
                return nullptr;
	        }
	    } else if (args[2].empty()){
	        if(JobsList::JobEntry * cmd = jobs.getJobById(stoi(args[1]))){
	            exeuteFgCommand(cmd->command->arguments);
                return nullptr;
	        }
	        else {
	            if (isNumber(args[1])){
	                cout <<"smash error: fg: job-id "<< args[1] <<" does not exist" <<endl;
	            }
	            else
	            cout << "smash error: fg: invalid arguments"<< endl;
	        }
	    }
	    else {
            cout << "smash error: fg: invalid arguments"<< endl;
	    }
	}
	return nullptr;
}
Command *SmallShell::CreateCommand(const char *cmd_line)
{
	vector<string> args = analyseTheLine(cmd_line);
	const bool is_in = BuiltinTable.find(args[0]) != BuiltinTable.end();
	if (is_in)
	{
		Command *comm = createBuiltInCommand(args);
		//         vector<string> tmp(20,"");
		//         args = tmp;
		return comm;
	}
	vector<string> tmp(20, "");
	args = tmp;
	return nullptr;
}
//Command * SmallShell::CreateCommand(const char* cmd_line) {
//	// For example:
//
//  string cmd_s = _trim(string(cmd_line));
//  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
//  if (firstWord.compare("pwd") == 0) {
//      string sec_word = cmd_s.substr(firstWord.size(), cmd_s.size());
//      if (sec_word.empty()) return new GetCurrDirCommand(cmd_line);
//      return nullptr;
//  }
//   if (firstWord.compare("showpid") == 0) {
//
//       string sec_word = cmd_s.substr(firstWord.size(), cmd_s.size());
//       if (sec_word.empty()) return new ShowPidCommand(cmd_line);
//    return nullptr;
//  }
//   if (firstWord.compare("cd") == 0) {
//       string sec_word = cmd_s.substr(firstWord.size(), cmd_s.size());
//       sec_word = _trim(sec_word);
//       string first_arg = sec_word.substr(0,sec_word.find_first_of(WHITESPACE));
//       string third_word = sec_word.substr(first_arg.size(), sec_word.size());
//       if ( first_arg.compare("-")==0){
//            if (paths.empty()) {
//                cout << "smash error: cd: OLDPWD not set "<<endl;
//                return nullptr;
//            }
//            else{
//
//            }
//       }
//       if ( ! third_word.empty()){
//           cout<<"smash error: cd: too many arguments"<<endl;
//           return nullptr;
//       }
////       paths[curr_path++]=sec_word;
//       return new ChangeDirCommand(cmd_line,first_arg);
//   }
//  return nullptr;
//}

void SmallShell::executeCommand(const char *cmd_line)
{
	// TODO: Add your implementation here

	Command *cmd = CreateCommand(cmd_line);
	if (!cmd)
		return;
	cmd->execute();
	delete cmd;
	// Please note that you must fork smash process for some commands (e.g., external commands....)
}

void ShowPidCommand::execute()
{
	int pid = getpid();
	cout << "smash pid is " << pid << endl;
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void GetCurrDirCommand::execute()
{
	char path[100];
	getcwd(path, 100);
	string str(path);
	cout << str << endl;
}

void ChangeDirCommand::execute()
{

	int res = chdir(path.c_str());
	if (res < 0)
	{
		cout << strerror(errno) << endl;
	}
}

void JobsList::removeFinishedJobs() {
    for (auto iterator=jobs.begin(); iterator != jobs.end(); iterator++ ){
        if(iterator->to_delete){
            jobs.erase(iterator);
        }
    }
    std::sort(jobs.begin(),jobs.end());
}

void JobsList::printJobsList() {
    removeFinishedJobs();
    for (auto iterator=jobs.begin(); iterator != jobs.end(); iterator++ ){
        string stop;
        time_t t1;
        time(&t1);
        Command* cmd =iterator->command;
        double time_elapsed = difftime(t1,iterator->timestamp);
        if (cmd->status == STOPPED) {
             stop = "(stopped)";
        }
        cout <<"["<<iterator->job_id<<"]";
        for (auto iterator1 = cmd->arguments.begin(); iterator1 != cmd->arguments.end();iterator1++){
            cout << *iterator1<<" ";
        }
        cout<<":"<<iterator->p_id <<" " << time_elapsed << " sec" <<stop << endl;
    }

}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    int i=0;
    int reIndex = -1;
    for (auto & job : jobs){
        if (job.command->status == STOPPED) {
            reIndex = i;
        }
        i++;
    }
    if (reIndex == -1)
    return nullptr;
    return &jobs[i];
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    return &jobs.back();
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    int i=0;
    int reIndex = -1;
    for (auto & job : jobs){
        if (job.job_id == jobId) {
            reIndex = i;
        }
        i++;
    }
    if (reIndex == -1)
        return nullptr;
    return &jobs[i];
}

void JobsList::removeJobById(int jobId) {
    JobEntry* jop = getJobById(jobId);
    jop->to_delete= true;

}

void JobsList::addJob(Command *cmd, bool isStopped) {
    JOB_TYPE type;
    if ( isStopped){
        type = STOP;
    }
    else{
        type = BACKGROUND;
    }
    if (vacant_ids.empty()){
        JobEntry jop(vacant_ids[0],getpid(),cmd,type);
        vacant_ids.erase(vacant_ids.begin());
        jobs.push_back(jop);
        return;
    }
    JobEntry jop(next_id,getpid(),cmd,type);
    vacant_ids.erase(vacant_ids.begin());
    jobs.push_back(jop);
    next_id++;
}

void JobsList::killAllJobs() {
    cout <<"smash: sending SIGKILL signal to "<<jobs.size()<< "jobs:"<<endl;
    for (auto & job : jobs){
      cout<<job.p_id<<":";
      for (auto & arg :job.command->arguments){
          cout <<arg;
      }
      cout << endl;
      kill(job.p_id,9);
    }
    jobs.clear();
    vacant_ids.clear();
    next_id=1;
}


void KillCommand::execute() {

     kill(jobsList->getJobById(job_id)->p_id,9);
     jobsList->removeJobById(job_id);
}
