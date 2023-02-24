#pragma once

#include "CoreDefines.h"
#include "Mem.h"

#include <string.h>

//#define CMP_KEY_FUNC(name) bool name(const void* A, const void* B)
//typedef CMP_KEY_FUNC(CompareKeyFunc);

template <typename tKey>
bool CompareKeys(const void* A, const void* B)
{
    return *(tKey*)A == *(tKey*)B;
}

// Good hash functions taken from here: https://nullprogram.com/blog/2018/07/31/ 
inline uint32 Hash32(uint32 x)
{
    x ^= x >> 15;
    x *= 0x2c1b3c6dU;
    x ^= x >> 12;
    x *= 0x297a2d39U;
    x ^= x >> 15;
    return x;
}

// Changed to return a uint32
inline uint32 Hash64(uint64 x)
{
    x ^= x >> 32;
    x *= 0xd6e8feb86659fd93U;
    x ^= x >> 32;
    x *= 0xd6e8feb86659fd93U;
    x ^= x >> 32;
    return x & 0xFFFFFFFF;
}

namespace Tk
{
namespace Core
{

// Open addressing hashmap
struct HashMapBase
{
    enum : uint32 { eInvalidIndex = MAX_UINT32 }; // index, not key
    
    TINKER_API ~HashMapBase();

private:
    uint32 ProbeFunc(uint32 index) const
    {
        return (index + 1) % m_size;
    }

protected:
    uint8* m_data;
    uint32 m_size;

    TINKER_API void Reserve(uint32 numEles, uint32 dataPairSize);
    TINKER_API void Clear(size_t dataPairSize);
    TINKER_API uint32 FindIndex(uint32 index, void* key, size_t dataPairSize, bool CompareKeysFunc(const void*, const void*), const void* m_InvalidKey) const;
    TINKER_API void* DataAtIndex(uint32 index, size_t dataPairSize, size_t dataValueOffset) const;
    TINKER_API void* KeyAtIndex(uint32 index, size_t dataPairSize) const;
    TINKER_API uint32 Insert(uint32 index, void* key, void* value, bool CompareKeysFunc(const void*, const void*), size_t dataPairSize, size_t dataValueOffset, size_t dataValueSize, const void* m_InvalidKey);
};

template <typename tKey, typename tVal, uint32 HashFunc(tKey)>
struct HashMap : public HashMapBase
{
private:
    struct Pair
    {
        tKey key;
        tVal value;
    };

    enum
    {
        ePairSize = sizeof(Pair),
        ePairValSize = sizeof(Pair::value),
        ePairValOffset = sizeof(Pair) - sizeof(Pair::value), // TODO: struct alignment issues?
    };

    tKey m_InvalidKey;

    uint32 Hash(tKey val, uint32 dataSizeMax) const
    {
        return HashFunc(val) % dataSizeMax;
    }

public:
    HashMap() : HashMapBase()
    {
        m_data = nullptr;
        m_size = 0;
        memset(&m_InvalidKey, 0xFF, sizeof(tKey));
    }

    tKey GetInvalidKey() const
    {
        return m_InvalidKey;
    }

    uint32 Size() const
    {
        return m_size;
    }
    
    void Reserve(uint32 numEles)
    {
        HashMapBase::Reserve(numEles, ePairSize);
    }

    void Clear()
    {
        HashMapBase::Clear(ePairSize);
    }

    uint32 FindIndex(tKey key) const
    {
        uint32 index = Hash(key, m_size);
        return HashMapBase::FindIndex(index, &key, ePairSize, CompareKeys<tKey>, &m_InvalidKey);
    }

    const tKey& KeyAtIndex(uint32 index) const
    {
        return *(tKey*)HashMapBase::KeyAtIndex(index, ePairSize);
    }

    const tVal& DataAtIndex(uint32 index) const
    {
        return *(tVal*)HashMapBase::DataAtIndex(index, ePairSize, ePairValOffset);
    }

    tVal& DataAtIndex(uint32 index)
    {
        return *(tVal*)HashMapBase::DataAtIndex(index, ePairSize, ePairValOffset);
    }

    uint32 Insert(tKey key, tVal value)
    {
        uint32 index = Hash(key, m_size);
        return HashMapBase::Insert(index, &key, &value, CompareKeys<tKey>, ePairSize, ePairValOffset, ePairValSize, &m_InvalidKey);
    }
};

// Common hashmap specializations
typedef HashMap<uint32, uint32, Hash32> HashMapU32;
typedef HashMap<uint64, uint64, Hash64> HashMapU64;

}
}
