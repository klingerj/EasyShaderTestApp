#pragma once

#define PERFORMANCE_TIMERS

#ifdef PERFORMANCE_TIMERS
#include "Logging.h"

#include <chrono>

#define MAX_MSG_LEN 128
#define MAX_TIME_DIGITS 16 + 1 // + 1 for the dot in a decimal
#define TIMER_MSG_SUFFIX " elapsed time in us: "
#define TIMER_MSG_SUFFIX_LEN sizeof(TIMER_MSG_SUFFIX) - 1 // don't copy null byte

namespace Tk
{
namespace Core
{
namespace Utility
{

struct ScopedTimer
{
private:
    char m_msg[MAX_MSG_LEN + TIMER_MSG_SUFFIX_LEN] = {}; // TODO: use better string system here
    uint32 m_msgSizeBeforeTimer;

    using Clock = std::chrono::steady_clock;
    std::chrono::time_point<Clock> m_startTime = {};
    char m_timeAsStr[MAX_TIME_DIGITS] = {};

public:
    template <uint32 strLen>
    ScopedTimer(const char(&msg)[strLen]) : m_msgSizeBeforeTimer(0)
    {
        const uint32 numCharsToCopy = strLen - 1; // don't copy null byte, assumes string literals
        static_assert(numCharsToCopy + TIMER_MSG_SUFFIX_LEN + MAX_TIME_DIGITS < MAX_MSG_LEN); // make sure final appended message will fit
        memcpy(m_msg, msg, numCharsToCopy);
        memcpy(m_msg + numCharsToCopy, TIMER_MSG_SUFFIX, TIMER_MSG_SUFFIX_LEN);
        m_msgSizeBeforeTimer = numCharsToCopy + TIMER_MSG_SUFFIX_LEN;

        m_startTime = Clock::now();
    }

    ~ScopedTimer()
    {
        auto currentTime = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>((currentTime - m_startTime));

        _ultoa_s((uint32)duration.count(), m_msg + m_msgSizeBeforeTimer, MAX_TIME_DIGITS, 10);
        Utility::LogMsg("Core - Performance", m_msg, Utility::LogSeverity::eInfo);
    }
};

}
}
}
#endif

// Don't compile any timers if we disable timing entirely
#ifdef PERFORMANCE_TIMERS
#define TIMED_SCOPED_BLOCK(msg) Tk::Core::Utility::ScopedTimer __LINE__##timer(msg);
#else
#define TIMED_SCOPED_BLOCK(msg)
#endif
