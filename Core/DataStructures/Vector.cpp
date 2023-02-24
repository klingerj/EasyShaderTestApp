#include "Vector.h"
#include "Mem.h"

#include <string.h>

namespace Tk
{
namespace Core
{

VectorBase::~VectorBase()
{
    CoreFree(m_data);
    m_data = nullptr;
    m_size = 0;
    m_capacity = 0;
}

void VectorBase::Reserve(uint32 numEles, uint32 eleSize)
{
    if (numEles > m_capacity)
    {
        const size_t BytesToAllocate = (size_t)numEles * (size_t)eleSize;
        TINKER_ASSERT(BytesToAllocate <= (size_t)MAX_UINT32);
        void* newData = CoreMalloc(numEles * eleSize);
        if (m_data && m_size > 0)
        {
            memcpy(newData, m_data, m_size * eleSize);
            CoreFree(m_data); // free old data
        }

        m_data = (uint8*)newData;
        m_capacity = numEles;
    }
}

void VectorBase::Resize(uint32 numEles, uint32 eleSize)
{
    Reserve(numEles, eleSize);
    if (m_size < numEles)
    {
        // Set new elements to 0
        memset(m_data + m_size * eleSize, 0, eleSize * (numEles - m_size));
        m_size = numEles;
    }
    else
    {
        m_size = numEles;
    }
}

void VectorBase::Clear()
{
    m_size = 0;
}

void VectorBase::PushBackRaw(void* data, uint32 eleSize)
{
    if (m_size == m_capacity)
    {
        // Alloc new data
        uint32 newCapacity = m_capacity + 1;
        void* newData = CoreMalloc(newCapacity * eleSize);
        memcpy(newData, m_data, m_capacity * eleSize);
        m_capacity = newCapacity;

        // Replace data
        CoreFree(m_data);
        m_data = (uint8*)newData;
    }

    if (m_size < m_capacity)
    {
        memcpy(m_data + m_size * eleSize, data, eleSize);
        ++m_size;
    }
}

uint32 VectorBase::Find(void* data, uint32 eleSize, CompareFunc Compare) const
{
    for (uint32 i = 0; i < m_size; ++i)
    {
        if (Compare(data, m_data + i * eleSize))
        {
            return i;
        }
    }

    return eInvalidIndex;
}

}
}
