#pragma once

#include "CoreDefines.h"
#include "Mem.h"

#include <string.h>

namespace Tk
{
namespace Core
{

#define CMP_LT_FUNC(name) bool name(const void* A, const void* B)
typedef CMP_LT_FUNC(CompareLessThan);

// Define custom compare funcs like so:
inline CMP_LT_FUNC(CompareLessThan_uint32)
{ 
    return *(uint32*)A < *(uint32*)B;
}

template <typename CmpFunc>
void Merge(uint8* data, uint32 numElesLeft, uint32 numElesRight, uint32 eleSize, CmpFunc Compare, const uint8* tmpList)
{
    uint32 totalEles = numElesLeft + numElesRight;

    uint32 i = 0, j = 0;
    while (1)
    {
        if (i + j == totalEles)
            break;

        // Assuming that left and right are actually contiguous
        uint8* dstList = data + (i + j) * eleSize;
        
        // Copy over left's next ele if i is not done counting AND either j is done, or ith < jth ele
        if ((j >= numElesRight || Compare(&tmpList[i * eleSize], &tmpList[(numElesLeft + j) * eleSize])) && i < numElesLeft)
        {
            memcpy(dstList, &tmpList[i * eleSize], eleSize);
            ++i;
        }
        else
        {
            memcpy(dstList, &tmpList[(numElesLeft + j) * eleSize], eleSize);
            ++j;
        }
    }
}

template <typename CmpFunc>
void MergeSortRecursive(uint8* data, uint32 numEles, uint32 eleSize, CmpFunc Compare, uint8* tmpList)
{
    if (numEles == 1)
        return;

    uint32 midpoint = numEles / 2;

    uint8* left = data;
    uint8* right = data + midpoint * eleSize;
    uint32 leftSize = midpoint;
    uint32 rightSize = numEles - midpoint;

    // Sort each half into temp array
    MergeSortRecursive(tmpList, leftSize, eleSize, Compare, left);
    MergeSortRecursive(tmpList + midpoint * eleSize, rightSize, eleSize, Compare, right);

    // Final merge into original array
    Merge(left, leftSize, rightSize, eleSize, Compare, tmpList);
}

template <typename T, typename CmpFunc>
void MergeSort(T* data, uint32 numEles, CmpFunc Compare)
{
    const uint32 eleSize = sizeof(T);
    // TODO: remove this temp alloc
    uint8* tmpList = (uint8*)CoreMalloc(numEles * eleSize);
    memcpy(tmpList, data, numEles * eleSize);
    MergeSortRecursive((uint8*)data, numEles, eleSize, Compare, tmpList);
    Tk::Core::CoreFree(tmpList);
}

}
}
