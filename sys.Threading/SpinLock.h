#pragma once

#include <atomic>

namespace sys
{
    struct lock_guard;

    class spin_lock
    {
        std::atomic_flag locked;
    public:
        void lock()
        {
            while (locked.test_and_set(std::memory_order_acquire));
        }
        bool tryLock()
        {
            return !locked.test_and_set(std::memory_order_acquire);
        }
        void unlock()
        {
            locked.clear(std::memory_order_release);
        }
    };

    struct lock_guard
    {
        explicit lock_guard(spin_lock& lock) : lock(lock)
        {
            this->lock.lock();
        }
        lock_guard(const lock_guard& other) = delete;
        lock_guard(lock_guard&& other) = delete;
        ~lock_guard()
        {
            this->lock.unlock();
        }

        lock_guard& operator=(const lock_guard& other) = delete;
        lock_guard& operator=(lock_guard&& other) = delete;
    private:
        spin_lock& lock; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    };
} // namespace sys
