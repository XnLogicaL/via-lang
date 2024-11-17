/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "magic_enum.hpp"
#include <cassert>
#include <print.h>

#define VIA_ASSERT(cond, err)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            std::cerr << "VIA_ASSERT(): " << (err) << "\n  in file " << __FILE__ << ", line " << __LINE__ << '\n';     \
            std::abort();                                                                                              \
        }                                                                                                              \
    } while (0)

#define ENUM_NAME(expr)    magic_enum::enum_name(expr)
#define ENUM_CAST(T, expr) magic_enum::enum_cast<T>(expr)
