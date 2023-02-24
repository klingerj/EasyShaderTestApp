#pragma once

#include "CoreDefines.h"
#include "Mem.h"

namespace Tk
{
namespace Core
{

struct LinearAllocator
{
    uint8* m_ownedMemPtr = nullptr;
    size_t m_capacity;
    size_t m_nextAllocOffset = 0;

    LinearAllocator() {}

    ~LinearAllocator()
    {
        ExplicitFree();
    }

    void ExplicitFree()
    {
        if (m_ownedMemPtr)
        {
            Tk::Core::CoreFreeAligned(m_ownedMemPtr);
            m_ownedMemPtr = nullptr;
        }
        m_nextAllocOffset = 0;
        m_capacity = 0;
    }

    void Init(size_t capacity, uint32 alignment)
    {
        TINKER_ASSERT(capacity > 0);
        m_capacity = capacity;
        m_ownedMemPtr = (uint8*)Tk::Core::CoreMallocAligned(m_capacity, alignment);
    }

    uint8* Alloc(size_t size, uint32 alignment)
    {
        TINKER_ASSERT(ISPOW2(alignment));

        size_t memPtrAsNum = (size_t)((uint8*)m_ownedMemPtr + m_nextAllocOffset);

        // If the current memory pointer is not aligned, add the alignment as offset
        size_t alignmentBits = LOG2(alignment);
        size_t alignedPtrAsNum = (memPtrAsNum >> alignmentBits) << alignmentBits;
        if (alignedPtrAsNum != memPtrAsNum)alignedPtrAsNum += alignment;
        
        // Check that there is room for this size allocation
        size_t allocSize = size + (alignedPtrAsNum - memPtrAsNum);
        if (allocSize > m_capacity - m_nextAllocOffset)
            return nullptr; // fail, no assert

        // Return new pointer
        uint8* newAllocPtr = (uint8*)alignedPtrAsNum;
        m_nextAllocOffset += allocSize;
        return newAllocPtr;
    }

    void ResetState()
    {
        m_nextAllocOffset = 0;
    }
};

template <typename T>
struct pool_element
{
    union
    {
        struct
        {
            T m_data;
        };
        struct
        {
            uint32 m_nextFreeEleIdx;
        };
    };
};

template <typename T, uint32 NumElements = 0, uint32 Alignment = 1>
struct PoolAllocator
{
    uint32 m_numAllocdElements = 0;
    uint32 m_maxPoolElements = 0;
    uint32 m_freeListHead = 0;

    template <typename U>
    using PoolElement = struct pool_element<U>;
    PoolElement<T>* m_pool = nullptr;
    uint32 m_elementSizeInBytes = sizeof(PoolElement<T>);

    PoolAllocator()
    {
        TINKER_ASSERT(ISPOW2(Alignment));
        if (NumElements > 0)
        {
            Init(NumElements, Alignment);
        }
        else
        {
            // User must specify alloc'd memory with Init()
        }
    }

    ~PoolAllocator()
    {
        ExplicitFree();
    }

    void ExplicitFree()
    {
        if (m_pool)
        {
            Tk::Core::CoreFreeAligned(m_pool);
            m_pool = nullptr;
            m_freeListHead = 0;
            m_maxPoolElements = 0;
        }
    }

    inline T* PtrFromHandle(uint32 handle)
    {
        return &m_pool[handle].m_data;
    }

    void Init(uint32 maxPoolElements, size_t alignment)
    {
        TINKER_ASSERT(m_maxPoolElements == 0);
        // Only call Init() if you did not provide the number of elements as a template at compile-time.

        TINKER_ASSERT(maxPoolElements > 0 && maxPoolElements < TINKER_INVALID_HANDLE);
        m_maxPoolElements = maxPoolElements;

        m_pool = (PoolElement<T>*)Tk::Core::CoreMallocAligned(m_maxPoolElements * m_elementSizeInBytes, alignment);

        // Init free list
        for (uint32 uiEle = 0; uiEle < m_maxPoolElements - 1; ++uiEle)
        {
            m_pool[uiEle].m_nextFreeEleIdx = uiEle + 1; // point to next element
        }
        m_pool[m_maxPoolElements - 1].m_nextFreeEleIdx = TINKER_INVALID_HANDLE;
    }

    uint32 Alloc()
    {
        if (m_freeListHead == TINKER_INVALID_HANDLE)
        {
            // Pool is full - resize?
            TINKER_ASSERT(0);
            return TINKER_INVALID_HANDLE;
        }
        else
        {
            uint32 newEle = m_freeListHead;
            m_freeListHead = m_pool[m_freeListHead].m_nextFreeEleIdx;
            ++m_numAllocdElements;

            if (m_numAllocdElements == m_maxPoolElements)
            {
                m_freeListHead = TINKER_INVALID_HANDLE;
            }

            return newEle;
        }
    }

    void Dealloc(uint32 handle)
    {
        TINKER_ASSERT(m_numAllocdElements > 0);
        TINKER_ASSERT(handle < m_maxPoolElements);
        m_pool[handle].m_nextFreeEleIdx = m_freeListHead;
        m_freeListHead = handle;
        --m_numAllocdElements;
    }
};

}
}
