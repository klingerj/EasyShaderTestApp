#include "Utility/Logging.h"

#include <windows.h>

namespace Tk
{
namespace Core
{
namespace Utility
{

void LogMsg(const char* prefix, const char* msg, uint32 severity)
{
    OutputDebugString("[");
    OutputDebugString(prefix);
    OutputDebugString("]");

    const char* severityMsg;
    switch (severity)
    {
        case LogSeverity::eInfo:
        {
            severityMsg = "Info";
            break;
        }
        case LogSeverity::eWarning:
        {
            severityMsg = "Warning";
            break;
        }
        case LogSeverity::eCritical:
        {
            severityMsg = "Critical";
            break;
        }
        default:
        {
            severityMsg = "Unknown Severity";
            break;
        }
    }
    OutputDebugString("[");
    OutputDebugString(severityMsg);
    OutputDebugString("] ");

    OutputDebugString(msg);
    OutputDebugString("\n");
}

}
}
}
