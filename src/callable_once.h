/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

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
