/// Based on: https://github.com/aappleby/smhasher and https://gist.github.com/oteguro/10538695

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

// NOTE(Joe) - this is the original function, modified slightly
uint32_t MurmurHash3_x86_32(const void* key, int len, uint32_t seed);

// NOTE(Joe) - what follows is the same murmur3 hash, but run at compile time for strings that it can be computed for (string literals)
inline consteval uint32_t rotl32_Internal(uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}
#define	ROTL32_INTERNAL(x,y) rotl32_Internal(x,y)

inline consteval uint32_t fmix32_Internal(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

static constexpr uint32_t c1_ = 0xcc9e2d51;
static constexpr uint32_t c2_ = 0x1b873593;

inline consteval uint32_t Load4_Internal(const char* data)
{
    return (uint32_t(data[3]) << 24) | (uint32_t(data[2]) << 16) |
        (uint32_t(data[1]) << 8) | (uint32_t(data[0]) << 0);
}

inline consteval uint32_t MurmurLoopTail_Internal(int len, const char* data, uint32_t h)
{
    uint32_t hash = h;
    uint32_t k1 = 0;

    switch (len & 3)
    {
    case 3: k1 ^= data[2] << 16;
    case 2: k1 ^= data[1] << 8;
    case 1: k1 ^= data[0];
        k1 *= c1_; k1 = ROTL32_INTERNAL(k1, 15); k1 *= c2_; hash ^= k1;
    }
    return hash;
}

inline consteval uint32_t MurmurBodyHash_Internal(uint32_t h, uint32_t data)
{
    uint32_t k1 = data;
    k1 *= c1_;
    k1 = ROTL32_INTERNAL(k1, 15);
    k1 *= c2_;

    h ^= k1;
    h = ROTL32_INTERNAL(h, 13);
    h = h * 5 + 0xe6546b64;
    return h;
}

inline consteval uint32_t MurmurLoopBody_Internal(int loopIters, int len, const char* data, uint32_t h)
{
    return (loopIters == 0 ?
        MurmurLoopTail_Internal(len, data, h) :
        MurmurLoopBody_Internal(loopIters - 1, len, data + 4, MurmurBodyHash_Internal(h, Load4_Internal(data))));
}

template <size_t N>
inline consteval uint32_t MurmurHash3_x86_32(const char (&key)[N], int len, uint32_t seed)
{
    uint32_t h1 = seed;
    h1 = MurmurLoopBody_Internal(len / 4, len, (const char*)key, h1);
    h1 ^= len;
    h1 = fmix32_Internal(h1);
    return h1;
}

//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_