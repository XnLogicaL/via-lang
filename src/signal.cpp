/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "signal.h"

namespace via::utils {

template<typename... Args>
void Signal<Args...>::Connection::disconnect()
{
    if (!active) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);
    slots.erase(slots.begin() + connection_id);
}

template<typename... Args>
Signal<Args...>::Connection Signal<Args...>::connect(const Slot &slot)
{
    std::lock_guard<std::mutex> lock(mutex);
    slots.push_back(slot);
    return Connection(slots, mutex, slots.size() - 1);
};

template<typename... Args>
void Signal<Args...>::fire(Args... args)
{
    std::lock_guard<std::mutex> lock(mutex);

    for (const Slot &slot : slots) {
        slot(args...);
    }

    condition.notify_all();
}

template<typename... Args>
void Signal<Args...>::wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock);
}

} // namespace via::utils
