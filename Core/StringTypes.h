#pragma once

#include "CoreDefines.h"
#include "MurmurHash3.h"

#include <cstring>

/* Murmur usage examples
// Compile time hash
uint32 testHash = MurmurHash3_x86_32("henlo ", 6, 0x54321);

// Run time hasn
const char* str = "henlo ";
uint32 testHash2 = MurmurHash3_x86_32(str, (int)strlen(str), 0x54321);
*/

namespace Tk
{
namespace Core
{

template <uint32 tLen>
struct StrFixedBuffer
{
    uint32 m_len = 0;
    char m_data[tLen] = {};

    uint32 LenRemaining()
    {
        return tLen - m_len;
    }

    char* EndOfStrPtr()
    {
        return &m_data[m_len];
    }

    void Clear()
    {
        m_len = 0;
        memset(m_data, 0, tLen);
    }

    void Append(const char* strToAppend, uint32 strToAppendLen)
    {
        if (VerifyNoOverflow(m_len, strToAppendLen))
        {
            memcpy(&m_data[m_len], strToAppend, strToAppendLen);
            m_len += strToAppendLen;
        }
    }

    void Append(const char* strToAppend)
    {
        uint32 appendLen = (uint32)strlen(strToAppend);
        Append(strToAppend, appendLen);
    }

    void AppendWChar(const wchar_t* strToAppend, uint32 strToAppendLen)
    {
        size_t numCharsWritten = 0;
        wcstombs_s(&numCharsWritten, NULL, 0, strToAppend, strToAppendLen); // returns number of bytes needed

        if (VerifyNoOverflow(m_len, (uint32)numCharsWritten))
        {
            uint32 maxBytesToWrite = min((uint32)numCharsWritten, tLen - m_len);
            wcstombs_s(&numCharsWritten, &m_data[m_len], maxBytesToWrite, strToAppend, strToAppendLen);

            /* Note: wcstombs_s will write a null terminator if one was not encountered in the src str.
               This string class will assume that the null terminator was not written. */
            m_len += (uint32)(numCharsWritten ? numCharsWritten - 1 : 0);
        }
    }

    void NullTerminate()
    {
        static const char nullTerm[1] = { '\0' };
        Append(&nullTerm[0], 1);
    }

private:
    uint32 VerifyNoOverflow(uint32 currLen, uint32 lenToAdd)
    {
        if (m_len + lenToAdd > tLen)
        {
            TINKER_ASSERT(0);
            return 0;
        }

        return 1;
    }
};

}
}
