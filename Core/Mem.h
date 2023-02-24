#pragma once

#include "CoreDefines.h"
#include "Utility/MemTracker.h"

namespace Tk
{
namespace Core
{

#ifndef ENABLE_MEM_TRACKING
TINKER_API void* CoreMalloc(size_t size);
#else
TINKER_API void* CoreMallocDbg(size_t size, const char* filename, int lineNum);
#define CoreMalloc(size) CoreMallocDbg(size, __FILE__, __LINE__)
#endif

TINKER_API void CoreFree(void* ptr);

#ifndef ENABLE_MEM_TRACKING
TINKER_API void* CoreMallocAligned(size_t size, size_t alignment);
#else
TINKER_API void* CoreMallocAlignedDbg(size_t size, size_t alignment, const char* filename, int lineNum);
#define CoreMallocAligned(size, alignment) CoreMallocAlignedDbg(size, alignment, __FILE__, __LINE__)
#endif

TINKER_API void CoreFreeAligned(void* ptr);

}
}
