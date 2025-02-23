// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#pragma once

#include "common.h"

#include <mutex>
#include <condition_variable>

namespace via::utils {

template<typename... Args>
class Signal {
public:
    using Slot = std::function<void(Args...)>;

    class Connection {
    public:
        Connection(std::vector<Slot> &_Slots, std::mutex &_Mutex, size_t _Connection_id)
            : slots(_Slots)
            , mutex(_Mutex)
            , connection_id(_Connection_id)
            , active(true)
        {
        }

        void disconnect();

    private:
        std::vector<Slot> &slots;
        std::mutex &mutex;

        size_t connection_id;
        bool active;
    };

    Connection connect(const Slot &);
    void fire(Args...);
    void wait();

private:
    std::vector<Slot> slots;
    std::mutex mutex;
    std::condition_variable condition;
};

} // namespace via::utils
