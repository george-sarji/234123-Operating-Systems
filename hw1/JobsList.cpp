#include "JobsList.h"

void JobsList::removeFinishedJobs()
{
    if (this->jobs.empty())
        return;
    int p_id;
    //    int w_status = 0;
    int w_res;
    for (auto iterator = jobs.begin(); iterator != jobs.end();)
    {
        p_id = iterator->p_id;
        w_res = waitpid(p_id, NULL, WNOHANG);
        if (w_res != 0)
        {
            jobs.erase(iterator);
            iterator = jobs.begin();
            vacant_ids.push_back(p_id);
            continue;
        }
        iterator++;
    }
    if (jobs.empty())
    {
        next_id = 1;
        return;
    }
    std::sort(jobs.begin(), jobs.end());
    next_id = jobs.back().job_id;
}

void JobsList::printJobsList()
{
    removeFinishedJobs();
    if (jobs.empty())
        return;
    for (auto &job : jobs)
    {
        string stop;
        time_t t1;
        time(&t1);
        string cmd = job.command;
        double time_elapsed = difftime(t1, job.timestamp);
        if (job.stopped)
        {
            stop = "(stopped)";
        }
        cout << "[" << job.job_id << "]";
        cout << job.command;
        cout << " :" << job.p_id << " " << time_elapsed << " sec" << stop << endl;
    }
}

JobsList::JobEntry *JobsList::getLastStoppedJob()
{
    int i = 0;
    int reIndex = -1;
    for (auto &job : jobs)
    {
        if (job.type == STOP)
        {
            reIndex = i;
        }
        i++;
    }
    if (reIndex == -1)
        return nullptr;
    return &jobs[i];
}

JobsList::JobEntry *JobsList::getLastJob()
{
    return &jobs.back();
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
    int i = 0;
    int reIndex = -1;
    for (auto &job : jobs)
    {
        if (job.job_id == jobId)
        {
            reIndex = i;
        }
        i++;
    }
    if (reIndex == -1)
        return nullptr;
    return &jobs[reIndex];
}

void JobsList::removeJobById(int jobId)
{
}

void JobsList::addJob(string cmd, pid_t p_id, bool isStopped)
{
    removeFinishedJobs();
    JOB_TYPE type;
    if (isStopped)
    {
        type = STOP;
    }
    else
    {
        type = BACKGROUND;
    }
    for (auto &job : jobs)
    {
        if (job.p_id == p_id)
        {
            job.stop();
            return;
        }
    }
    int id;
    if (jobs.empty())
        id = 1;
    else
        id = jobs.back().job_id + 1;
    JobEntry jop(id, p_id, std::move(cmd), type,isStopped);
    jobs.push_back(jop);
    sort(jobs.begin(), jobs.end());
}

void JobsList::killAllJobs()
{
    cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << endl;
    for (auto &job : jobs)
    {
        cout << job.p_id << ":";
        cout << job.command;
        cout << endl;
        kill(job.p_id, 9);
    }
    jobs.clear();
    vacant_ids.clear();
    next_id = 1;
}
