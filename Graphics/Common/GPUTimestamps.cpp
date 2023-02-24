#include "GPUTimestamps.h"
#include "GraphicsCommon.h"

namespace Tk
{
namespace Graphics
{
namespace GPUTimestamps
{

static uint64 gpuTimestampCPUCopy         [GPU_TIMESTAMP_NUM_MAX];

static uint32      numGPUTimestampsRecorded [MAX_FRAMES_IN_FLIGHT] = {};
static const char* gpuTimestampNamesRecorded[MAX_FRAMES_IN_FLIGHT][GPU_TIMESTAMP_NUM_MAX] = {};

static Timestamp timestampDataProcessed[GPU_TIMESTAMP_NUM_MAX - 1] = {};
static float totalTimeThisFrameInUS = 0.0f;

void RecordName(const char* timestampName)
{
    uint32 currentFrame = GetCurrentFrameInFlightIndex();

    gpuTimestampNamesRecorded[currentFrame][numGPUTimestampsRecorded[currentFrame]] = timestampName;
    ++numGPUTimestampsRecorded[currentFrame];
    TINKER_ASSERT(numGPUTimestampsRecorded[currentFrame] <= GPU_TIMESTAMP_NUM_MAX);
}

void* GetRawCPUSideTimestampBuffer()
{
    return gpuTimestampCPUCopy;
}

uint32 GetMostRecentRecordedTimestampCount()
{
    return numGPUTimestampsRecorded[GetCurrentFrameInFlightIndex()];
}

void ProcessTimestamps()
{
    uint32 currentFrame = GetCurrentFrameInFlightIndex();
    uint32 numTimestamps = numGPUTimestampsRecorded[currentFrame];

    if (numTimestamps < 1)
    {
        return;
    }

    // Convert ticks to time
    const double microsecondsPerTick = 1e-3 * GetGPUTimestampPeriod();

    // Calc total time
    totalTimeThisFrameInUS = (float)(microsecondsPerTick * (double)(gpuTimestampCPUCopy[numTimestamps - 1] - gpuTimestampCPUCopy[0]));

    for (uint32 i = 1; i < numTimestamps; ++i)
    {
        timestampDataProcessed[i - 1].timeInst = (float)(microsecondsPerTick * (double)(gpuTimestampCPUCopy[i] - gpuTimestampCPUCopy[i - 1]));
        timestampDataProcessed[i - 1].name = gpuTimestampNamesRecorded[currentFrame][i];
    }

    // reset
    numGPUTimestampsRecorded[currentFrame] = 0;
}

TimestampData GetTimestampData()
{
    uint32 numTimestamps = numGPUTimestampsRecorded[GetCurrentFrameInFlightIndex()];
    if (numTimestamps > 0)
    {
        // We assume the game has recorded a "begin frame" marker that we effectively ignore after calculating differences
        --numTimestamps;
    }

    TimestampData data;
    data.timestamps = timestampDataProcessed;
    data.numTimestamps = numTimestamps;
    data.totalFrameTimeInUS = totalTimeThisFrameInUS;
    return data;
}

}
}
}
