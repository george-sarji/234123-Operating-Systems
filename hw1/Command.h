#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#if 0
#define FUNC_ENTRY() \
    cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
    cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#include <vector>
#include <string>
using namespace std;

enum COMMAND_STATUS
{
    ACTIVE,
    STOPPED,
    FINISH
};

class Command
{
public:
    const char *cmd_line;
    COMMAND_STATUS status;
    int job_id;
    std::vector<string> arguments;
    // TODO: Add your data members

    Command(const char *cmd_line) : cmd_line(cmd_line) {}
    virtual ~Command() {}
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

#endif