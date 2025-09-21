/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>

#define PASTE(a, b) a##b
#define STRING(X) #X

#define EXPAND(X) X
#define EXPAND_STRING(X) STRING(X)
#define EXPAND_AND_PASTE(A, B) PASTE(A, B)

// xmacro utils
#define DEFINE_ENUM(OP) OP,

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
