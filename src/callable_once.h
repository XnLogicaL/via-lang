// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via::utils {

template<typename T, typename... K>
class CallableOnce {
public:
    CallableOnce(std::function<T(K...)>);

    bool was_called() noexcept;
    std::optional<T> call_s(K...) noexcept;
    T call(K...);

private:
    bool called = false;
    std::function<T(K...)> func;
};

} // namespace via::utils
