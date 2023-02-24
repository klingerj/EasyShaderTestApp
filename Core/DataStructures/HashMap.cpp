#include "HashMap.h"
#include "Mem.h"

#include <string.h>

namespace Tk
{
namespace Core
{

TINKER_API HashMapBase::~HashMapBase()
{
    CoreFree(m_data);
    m_data = nullptr;
    m_size = 0;
}

TINKER_API void HashMapBase::Reserve(uint32 numEles, uint32 dataPairSize)
{
    if (numEles > m_size)
    {
        const size_t BytesToAllocate = (size_t)numEles * (size_t)dataPairSize;
        TINKER_ASSERT(BytesToAllocate <= (size_t)MAX_UINT32);
        void* newData = CoreMalloc(BytesToAllocate);

        if (m_data && m_size > 0)
        {
            memcpy(newData, m_data, m_size * dataPairSize);
            CoreFree(m_data); // free old data
        }

        m_data = (uint8*)newData;

        // Init all other elements to invalid
        uint32 numRemainingEles = numEles - m_size;
        memset(m_data + m_size * dataPairSize, 0xFF, numRemainingEles * dataPairSize);

        m_size = numEles;
    }
}

TINKER_API void HashMapBase::Clear(size_t dataPairSize)
{
    memset(m_data, 0xFF, m_size * dataPairSize);
}

TINKER_API uint32 HashMapBase::FindIndex(uint32 index, void* key, size_t dataPairSize, bool CompareKeysFunc(const void*, const void*), const void* m_InvalidKey) const
{
    if (CompareKeysFunc(key, m_InvalidKey))
        return eInvalidIndex;

    uint32 currIndex = index;
    do
    {
        void* dataKey  = m_data + currIndex * dataPairSize;
        if (CompareKeysFunc(dataKey, key))
        {
            return currIndex;
        }

        currIndex = ProbeFunc(currIndex);
    } while (currIndex != index);

    return eInvalidIndex;
}

TINKER_API void* HashMapBase::DataAtIndex(uint32 index, size_t dataPairSize, size_t dataValueOffset) const
{
    TINKER_ASSERT(index < m_size);
    return m_data + index * dataPairSize + dataValueOffset;
}

TINKER_API void* HashMapBase::KeyAtIndex(uint32 index, size_t dataPairSize) const
{
    TINKER_ASSERT(index < m_size);
    return m_data + index * dataPairSize;
}

TINKER_API uint32 HashMapBase::Insert(uint32 index, void* key, void* value, bool CompareKeysFunc(const void*, const void*), size_t dataPairSize, size_t dataValueOffset, size_t dataValueSize, const void* m_InvalidKey)
{
    if (CompareKeysFunc(key, m_InvalidKey))
        return eInvalidIndex;

    uint32 currIndex = index;
    do
    {
        void* keyToInsertAt = m_data + currIndex * dataPairSize;

        // check if key is marked as invalid (unused) or matches the input key
        if (CompareKeysFunc(keyToInsertAt, m_InvalidKey) || CompareKeysFunc(keyToInsertAt, key))
        {
            // found a slot
            memcpy(keyToInsertAt, key, dataValueOffset); // write key - assumes that offset is the same as key size
            memcpy((uint8*)keyToInsertAt + dataValueOffset, value, dataValueSize); // write value
            return currIndex;
        }
        else
        {
            currIndex = ProbeFunc(currIndex);
        }
    } while (currIndex != index);

    return eInvalidIndex;
}

}
}
