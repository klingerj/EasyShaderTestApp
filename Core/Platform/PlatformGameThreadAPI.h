#pragma once

#include "CoreDefines.h"
#include "Mem.h"

namespace Tk
{
namespace Platform
{

struct  WorkerJob
{
public:
    alignas(CACHE_LINE) volatile uint32 m_done = 0;
    virtual ~WorkerJob() {}

    virtual void operator()() = 0;
};

template <typename T>
struct JobFunc : public WorkerJob
{
public:
    virtual ~JobFunc() {}

    alignas(CACHE_LINE) T m_func;

    JobFunc(T func) : m_func(func) {}
    void operator()() override { m_func(); };
};

inline void WaitOnJob(WorkerJob* job)
{
    while (!job->m_done);
}

struct WorkerJobList
{
public:
    uint32 m_numJobs;
    WorkerJob* m_jobs[256]; // TODO: make this a templated sized array or something

    WorkerJobList() : m_numJobs(0) {}
    ~WorkerJobList() {}

    void Init(uint32 numJobs)
    {
        m_numJobs = numJobs;
        for (uint32 i = 0; i < ARRAYCOUNT(m_jobs); ++i)
        {
            m_jobs[i] = nullptr;
        }
    }

    void FreeList()
    {
        if (m_jobs)
        {
            for (uint32 i = 0; i < m_numJobs; ++i)
            {
                if (m_jobs[i])
                {
                    m_jobs[i]->~WorkerJob();
                    Tk::Core::CoreFreeAligned(m_jobs[i]);
                }
            }
        }
    }

    void WaitOnJobs()
    {
        for (uint32 i = 0; i < m_numJobs; ++i)
        {
            if (m_jobs[i])
            {
                WaitOnJob(m_jobs[i]);
            }
        }
    }
};

template <typename T>
WorkerJob* CreateNewThreadJob(T t)
{
    uint8* NewJobMem = (uint8*)Tk::Core::CoreMallocAligned(sizeof(JobFunc<T>), CACHE_LINE);
    JobFunc<T>* NewJob = new (NewJobMem) JobFunc<T>(t);
    NewJob->m_done = 0;
    return NewJob;
}

#define ENQUEUE_WORKER_THREAD_JOB(name) TINKER_API void name(WorkerJob* Job)
ENQUEUE_WORKER_THREAD_JOB(EnqueueWorkerThreadJob);

#define ENQUEUE_WORKER_THREAD_JOB_LIST(name) TINKER_API void name(WorkerJobList* JobList)
ENQUEUE_WORKER_THREAD_JOB_LIST(EnqueueWorkerThreadJobList_Unassisted);
ENQUEUE_WORKER_THREAD_JOB_LIST(EnqueueWorkerThreadJobList_Assisted);

}
}
