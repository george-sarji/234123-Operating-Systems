#ifndef SMASH_JOBSLIST_H_
#define SMASH_JOBSLIST_H_

using namespace std;

#include <string>
#include <vector>
#include <csignal>
#include <algorithm>
#include <sys/wait.h>
#include <iostream>
#include "time.h"

enum JOB_TYPE
{
    STOP,
    BACKGROUND
};

class JobsList
{
public:
    class JobEntry
    {

        /*
			Member descriptions
			job_id		Current job ID
			p_id		Current process ID
			background	Boolean if the process is background or not
			to_delete	Flag to check whether to delete process or not
			stopped		Activity status of process
			inserted 	Time (in seconds) when the job was first inserted
		*/
    public:
        JobEntry(int Jop_id, int p_id, string command1, JOB_TYPE jobType) : job_id(Jop_id), p_id(p_id), stopped(false),
                                                                            timestamp(), type(jobType), command(command1)
        {
            time(&timestamp);
        }
        int job_id, p_id;
        bool stopped;
        time_t timestamp;
        JOB_TYPE type;
        string command;

        void stop()
        {
            this->stopped = true;
            time(&timestamp);
        }

        void _continue_()
        {
            this->stopped = false;
            kill(this->p_id, SIGCONT);
        }

        bool operator==(const JobEntry &jobEntry) const
        {
            return job_id == jobEntry.job_id && p_id == jobEntry.p_id;
        }

        bool operator!=(const JobEntry &jobEntry) const
        {
            return !this->operator==(jobEntry);
        }

        bool operator>(const JobEntry &jobEntry) const
        {
            return job_id > jobEntry.job_id;
        }

        bool operator<(const JobEntry &jobEntry) const
        {
            return !this->operator>(jobEntry);
        }

        // TODO: Add your data members
    };

private:
    // TODO: Add your data members
    int next_id;
    std::vector<JobEntry> jobs;
    std::vector<int> vacant_ids;
    int size = jobs.size();

public:
    JobsList() : next_id(1), jobs(), vacant_ids(), size() {}
    ~JobsList() = default;
    void addJob(string command, pid_t pid, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry *getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry *getLastJob();
    JobEntry *getLastStoppedJob();
    bool empty()
    {
        return size == 0;
    }
    // TODO: Add extra methods or modify exisitng ones as needed
};

#endif