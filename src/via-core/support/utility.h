/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <utility>
#include <via/config.h>
#include <via/types.h>

#define PASTE(a, b) a##b
#define STRING(X) #X

#define EXPAND(X) X
#define EXPAND_STRING(X) STRING(X)
#define EXPAND_AND_PASTE(A, B) PASTE(A, B)

// xmacro utils
#define DEFINE_ENUM(OP) OP,
#define DEFINE_CASE(OP, ...)                                                             \
    case OP:                                                                             \
        __VA_ARGS__
#define DEFINE_CASE_TO_STRING(OP)                                                        \
    case OP:                                                                             \
        return EXPAND_STRING(OP);

// Enum utils
#define DEFINE_TO_STRING(ENUM, ...)                                                      \
    constexpr const char* to_string(ENUM val) noexcept                                   \
    {                                                                                    \
        using enum ENUM;                                                                 \
        switch (val) {                                                                   \
            __VA_ARGS__                                                                  \
        default:                                                                         \
            std::unreachable();                                                          \
        }                                                                                \
    }

#define NO_COPY(TARGET)                                                                  \
    TARGET& operator=(const TARGET&) = delete;                                           \
    TARGET(const TARGET&) = delete;

#define IMPL_COPY(TARGET)                                                                \
    TARGET& operator=(const TARGET&);                                                    \
    TARGET(const TARGET&);

#define NO_MOVE(TARGET)                                                                  \
    TARGET& operator=(TARGET&&) = delete;                                                \
    TARGET(TARGET&&) = delete;

#define IMPL_MOVE(TARGET)                                                                \
    TARGET& operator=(TARGET&&);                                                         \
    TARGET(TARGET&&);
