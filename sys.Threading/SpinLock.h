#pragma once

#include <atomic>

namespace sys
{
    struct lock_guard;

    class spin_lock
    {
        std::atomic_flag locked = ATOMIC_FLAG_INIT;
    public:
        inline void lock()
        {
            while (locked.test_and_set(std::memory_order_acquire));
        }
        inline bool tryLock()
        {
            return !locked.test_and_set(std::memory_order_acquire);
        }
        inline void unlock()
        {
            locked.clear(std::memory_order_release);
        }

        inline lock_guard guard();
    };

    struct lock_guard
    {
        inline lock_guard(spin_lock& lock) : lock(lock)
        {
            this->lock.lock();
        }
        inline ~lock_guard()
        {
            this->lock.unlock();
        }
    private:
        spin_lock& lock;
    };

    lock_guard spin_lock::guard()
    {
        return lock_guard(*this);
    }
} // namespace sys