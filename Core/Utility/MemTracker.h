#pragma once

#include "CoreDefines.h"

#define MAX_RECORDS MAX_UINT16

namespace Tk
{
namespace Core
{
namespace Utility
{

// TODO: make this stuff thread-safe
void RecordMemAlloc(uint64 sizeInBytes, void* memPtr, const char* filename, int lineNum);
void RecordMemDealloc(void* memPtr);
void DebugOutputAllMemAllocs();

}
}
}
