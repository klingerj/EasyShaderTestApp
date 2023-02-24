#pragma once

#include "CoreDefines.h"

namespace Tk
{
namespace Graphics
{

namespace GPUTimestamps
{
    struct Timestamp
    {
        float timeInst;
        const char* name;
    };

    struct TimestampData
    {
        Timestamp* timestamps;
        uint32 numTimestamps;
        float totalFrameTimeInUS;
    };

    void RecordName(const char* timestampName);
    void* GetRawCPUSideTimestampBuffer();
    uint32 GetMostRecentRecordedTimestampCount();
    void ProcessTimestamps();

    TimestampData GetTimestampData();
}

}
}