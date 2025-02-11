/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

#include <mutex>
#include <condition_variable>

namespace via::utils
{

template<typename... Args>
class Signal
{
public:
    using Slot = std::function<void(Args...)>;
    class Connection
    {
    public:
        Connection(std::vector<Slot> &_Slots, std::mutex &_Mutex, size_t _Connection_id)
            : slots(_Slots)
            , mutex(_Mutex)
            , connection_id(_Connection_id)
            , active(true)
        {
        }

        VIA_INLINE void disconnect()
        {
            if (!active)
                return;

            std::lock_guard<std::mutex> lock(mutex);
            slots.erase(slots.begin() + connection_id);
        }

    private:
        std::vector<Slot> &slots;
        std::mutex &mutex;
        size_t connection_id;
        bool active;
    };

    VIA_INLINE Connection connect(const Slot &slot)
    {
        std::lock_guard<std::mutex> lock(mutex);
        slots.push_back(slot);
        return Connection(slots, mutex, slots.size() - 1);
    };

    VIA_INLINE void fire(Args... args)
    {
        std::lock_guard<std::mutex> lock(mutex);

        for (const Slot &slot : slots)
            slot(args...);

        condition.notify_all();
    }

    VIA_INLINE void wait()
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock);
    }

private:
    std::vector<Slot> slots;
    std::mutex mutex;
    std::condition_variable condition;
};

} // namespace via::utils
