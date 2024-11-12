#ifndef VIA_CORE_H
#define VIA_CORE_H

#include "common.h"
#include <cassert>
#include "magic_enum.hpp"

#define VIA_ASSERT(cond, err) \
    do { \
        if (!(cond)) { \
            std::cerr << "VIA_ASSERT(): " << (err) \
                      << "\n  in file " << __FILE__ \
                      << ", line " << __LINE__ << '\n'; \
            std::abort(); \
        } \
    } while (0)

#define ENUM_NAME(expr) magic_enum::enum_name(expr)
#define ENUM_CAST(T, expr) magic_enum::enum_cast<T>(expr)

#endif // VIA_CORE_H
