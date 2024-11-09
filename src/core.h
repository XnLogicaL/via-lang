#ifndef VIA_CORE_H
#define VIA_CORE_H

#include "common.h"
#include <cassert>

#define VIA_ASSERT(cond, err) \
    do { \
        if (!(cond)) { \
            std::cerr << "VIA_ASSERT(): " << (err) \
                      << "\n  in file " << __FILE__ \
                      << ", line " << __LINE__ << '\n'; \
            std::abort(); \
        } \
    } while (0)

#endif // VIA_CORE_H
