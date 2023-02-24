#include "Win32WorkerThreadPool.h"
#include "PlatformGameAPI.h"

#include <process.h>
#include <windows.h>
#include <emmintrin.h>

#define NUM_JOBS_PER_WORKER 512
#define WORKER_THREAD_STACK_SIZE 1024 * 1024 * 2
#define MAX_THREADS 16u

namespace Tk
{
namespace Platform
{
namespace ThreadPool
{
typedef struct thread_info
{
    alignas(CACHE_LINE) volatile uint32 terminate = 0;
    volatile bool didTerminate = 1;
    uint32 threadId = 0;
    alignas(CACHE_LINE) uint64 semaphoreHandle = TINKER_INVALID_HANDLE;
    alignas(CACHE_LINE) Core::RingBuffer<WorkerJob*> jobs;
} ThreadInfo;

static ThreadInfo g_Threads[MAX_THREADS];
static volatile uint32 g_NumThreads = 0;
static volatile uint32 g_SchedulerCounter = 0;

uint32 NumWorkerThreads()
{
    return g_NumThreads;
}

void __cdecl WorkerThreadFunction(void* arg)
{
    ThreadInfo* info = (ThreadInfo*)(arg);

    //uint64 processorAffinityMask = 1ULL << (info->threadId * 2 + 1);
    //SetThreadAffinityMask(GetCurrentThread(), processorAffinityMask);
    //SetThreadIdealProcessor(GetCurrentThread(), info->threadId + 1);
    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

outer_loop:
    while (!info->terminate)
    {
        const uint32 limit = 4096;
        uint32 count = 0;

        while (count++ < limit)
        {
            uint32 head = info->jobs.m_head.load(std::memory_order_acquire);
            uint32 tail = info->jobs.m_tail.load(std::memory_order_acquire);

            if (head - tail > 0)
            {
                WorkerJob* job;
                info->jobs.Dequeue(&job);
                (*job)();
                job->m_done = 1;
                goto outer_loop; // reset counter until sema
            }
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
        }

        WaitForSingleObjectEx((HANDLE)info->semaphoreHandle, INFINITE, FALSE);
    }

    info->didTerminate = 1;
}

void Startup(uint32 NumThreads)
{
    g_NumThreads = Min(NumThreads, MAX_THREADS);
    for (uint32 i = 0; i < g_NumThreads; ++i)
    {
        g_Threads[i].jobs.Init(NUM_JOBS_PER_WORKER);
        g_Threads[i].terminate = 0;
        g_Threads[i].didTerminate = 0;
        g_Threads[i].threadId = i;
        g_Threads[i].semaphoreHandle = (uint64)CreateSemaphoreEx(0, 0, NUM_JOBS_PER_WORKER, 0, 0, SEMAPHORE_ALL_ACCESS);
        _beginthread(WorkerThreadFunction, WORKER_THREAD_STACK_SIZE, &g_Threads[i]);
    }
}

void Shutdown()
{
    // Tell the threads to wake up and terminate ASAP
    for (uint32 i = 0; i < g_NumThreads; ++i)
    {
        g_Threads[i].terminate = 1;
        ReleaseSemaphore((HANDLE)g_Threads[i].semaphoreHandle, 1, 0);
    }
    // Wait for the threads to finish their current tasks, then terminate
    for (uint32 i = 0; i < g_NumThreads; ++i)
    {
        while(!g_Threads[i].didTerminate);
    }

    // Free the job buffers
    for (uint32 i = 0; i < g_NumThreads; ++i)
    {
        g_Threads[i].jobs.ExplicitFree();
    }
}

void EnqueueSingleJob(WorkerJob* Job)
{
    g_Threads[g_SchedulerCounter].jobs.Enqueue(Job);
    ReleaseSemaphore((HANDLE)g_Threads[g_SchedulerCounter].semaphoreHandle, 1, 0);
    g_SchedulerCounter = (g_SchedulerCounter + 1) % g_NumThreads;
}

void EnqueueJobList(WorkerJobList* JobList)
{
    uint32 NumJobs = JobList->m_numJobs;
    for (uint32 uiJob = 0; uiJob < NumJobs; ++uiJob)
    {
        EnqueueSingleJob(JobList->m_jobs[uiJob]);
    }
}

void EnqueueJobSubList(WorkerJobList* JobList, uint32 NumJobs)
{
    for (uint32 uiJob = 0; uiJob < NumJobs; ++uiJob)
    {
        EnqueueSingleJob(JobList->m_jobs[uiJob]);
    }
}

}
}
}
