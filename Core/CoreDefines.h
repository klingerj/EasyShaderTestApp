#pragma once

#include <stdint.h>
#include <math.h>
#include <cmath>

#ifdef ASSERTS_ENABLE
#include <assert.h>
#define TINKER_ASSERT(cond) assert((cond))
#else
#define TINKER_ASSERT(cond) 
#endif

#define CACHE_LINE 64

// C++ only!
#define RESTRICT __restrict
#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

#define MAX_UINT64 0xFFFFFFFFFFFFFFFF
#define MAX_UINT32 0xFFFFFFFF
#define MAX_UINT16 0xFFFF

#define TINKER_INVALID_HANDLE MAX_UINT32

template <typename T>
T Max(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

template <typename T>
T Min(const T& a, const T& b)
{
    return (a < b) ? a : b;
}
#define CLAMP(a, b, c) Min(Max(a, b), c)

template <typename T>
bool ISPOW2(const T& x)
{
    return x && ((x & (x - 1)) == 0);
}

template <typename T>
T LOG2(T x)
{
    T i = 1;
    for (; x >> i; ++i) {}
    return i;
}

inline uint32 POW2(uint32 x)
{
    return 1 << LOG2(x);
}

// TODO: remove usage of this
inline uint32 POW2_ROUNDUP(uint32 x)
{
    return ISPOW2(x) ? x : POW2(x);
}

template <typename T>
T RoundValueToPow2(T val, T pow2)
{
    return (val + pow2 - 1) & ~(pow2 - 1);
}

#define FLOAT_EQUAL(a, b) fabs(a - b) < FLT_EPSILON

inline uint32 SafeTruncateUint64(uint64 value)
{
    TINKER_ASSERT(value <= 0xffffffff);
    return (uint32)value;
}

#define ARRAYCOUNT(arr) (sizeof((arr)) / sizeof((arr)[0]))

#ifdef _WIN32
#define OPTIMIZATIONS_ON __pragma(optimize( "", on ))
#define OPTIMIZATIONS_OFF __pragma(optimize( "", off ))
#endif

#if defined(_WIN32) && (defined(TINKER_APP) || defined(TINKER_GAME))
#ifdef TINKER_EXPORTING
#define TINKER_API __declspec(dllexport)
#else
#define TINKER_API __declspec(dllimport)
#endif
#else
#define TINKER_API
#endif
