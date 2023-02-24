#pragma once

#include "CoreDefines.h"

namespace Tk
{
namespace Core
{
namespace Utility
{

namespace LogSeverity
{
    enum : uint32
    {
        eInfo = 0,
        eWarning,
        eCritical
    };
}

TINKER_API void LogMsg(const char* prefix, const char* msg, uint32 severity);

}
}
}
