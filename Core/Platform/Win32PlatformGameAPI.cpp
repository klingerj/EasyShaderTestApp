#include "PlatformGameAPI.h"

#include <windows.h>

namespace Tk
{
namespace Platform
{

// I/O
PRINT_DEBUG_STRING(PrintDebugString)
{
    OutputDebugString(str);
}

ALLOC_ALIGNED_RAW(AllocAlignedRaw)
{
    return _aligned_malloc(size, alignment);
}

FREE_ALIGNED_RAW(FreeAlignedRaw)
{
    _aligned_free(ptr);
}

}
}
