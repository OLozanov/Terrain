#pragma once

#include <atomic>

class Event
{
    std::atomic<bool> m_flag = false;

public:
    Event() = default;

    void wait() noexcept
    {
        while (m_flag != true) ;
        m_flag = false;
    }

    void signal() noexcept { m_flag = true; }
};

class SpinLock
{
    std::atomic_flag m_flag = ATOMIC_FLAG_INIT;

public:
    void lock() noexcept
    {
        while (m_flag.test_and_set()) ;
    }

    void unlock() noexcept { m_flag.clear(); }
};