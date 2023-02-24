#pragma once

#include "CoreDefines.h"
#include "Mem.h"

#include <atomic>
#include <xmmintrin.h>

namespace Tk
{
namespace Core
{

// Single Producer Single Consumer wait-free queue
template <typename T, uint32 size = 0> // TODO: assert Size >= 1? or that it is a power of 2?
struct RingBuffer
{
private:
    uint32 _SIZE;
    uint32 _MASK;
public:
    T* m_data = nullptr;
    alignas(CACHE_LINE) volatile std::atomic<uint32> m_head = 0;
    alignas(CACHE_LINE) volatile std::atomic<uint32> m_tail = 0;

    RingBuffer()
    {
        if (size > 0)
        {
            Init(size);
        }
        else
        {
            // User must specify size with Init()
        }
        
    }

    ~RingBuffer()
    {
        ExplicitFree();
    }

    void ExplicitFree()
    {
        if (m_data)
        {
            Tk::Core::CoreFreeAligned(m_data);
            m_data = nullptr;
            _SIZE = 0;
            _MASK = 0;
        }
    }

    void Init(uint32 size)
    {
        _SIZE = POW2_ROUNDUP(size);
        _MASK = _SIZE - 1;
        m_data = (T*)Tk::Core::CoreMallocAligned(_SIZE * sizeof(T), CACHE_LINE);
    }

    uint32 Capacity() const
    {
        return _SIZE;
    }

    void Clear()
    {
        m_head = 0;
        m_tail = 0;
    }

    // Should only be called from the producer
    void Enqueue(T ele)
    {
        uint32 head = std::atomic_load_explicit(&m_head, std::memory_order_acquire);
        m_data[head & _MASK] = ele;
        
        std::atomic_thread_fence(std::memory_order_release);
        _mm_sfence();

        atomic_fetch_add(&m_head, 1);
    }

    // Should only be called from the consumer
    void Dequeue(T* ele)
    {
        uint32 tail = std::atomic_load_explicit(&m_tail, std::memory_order_acquire);
        *ele = m_data[tail & _MASK];

        std::atomic_thread_fence(std::memory_order_acquire);

        atomic_fetch_add(&m_tail, 1);
    }

    // Can be called from either producer or consumer
    uint32 Size()
    {
        const uint32 head = atomic_load_explicit(&m_head, std::memory_order_acquire);
        const uint32 tail = atomic_load_explicit(&m_tail, std::memory_order_acquire);
        return head - tail;
    }
};

}
}
