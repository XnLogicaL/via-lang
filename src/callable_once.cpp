// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "callable_once.h"

namespace via::utils {

template<typename T, typename... K>
CallableOnce<T, K...>::CallableOnce(std::function<T(K...)> func)
    : func(func)
{
}

template<typename T, typename... K>
bool CallableOnce<T, K...>::was_called() noexcept
{
    return called;
}

template<typename T, typename... K>
std::optional<T> CallableOnce<T, K...>::call_s(K... args) noexcept
{
    if (was_called()) {
        return std::nullopt;
    }

    called = true;
    return func(args...);
}

template<typename T, typename... K>
T CallableOnce<T, K...>::call(K... args)
{
    if (was_called()) {
        throw std::bad_function_call();
    }

    called = true;
    return func(args...);
}

} // namespace via::utils
