/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

#define TAG_ARGUMENT(ptr) (reinterpret_cast<uintptr_t>(ptr) | 0x1)
#define TAG_RETURN_VALUE(ptr) (reinterpret_cast<uintptr_t>(ptr) | 0x2)
#define TAG_STACK_FRAME(ptr) (reinterpret_cast<uintptr_t>(ptr) | 0x4)

#define IS_ARGUMENT(ptr) (!(reinterpret_cast<uintptr_t>(ptr) & 0x1))
#define IS_RETURN_VALUE(ptr) (reinterpret_cast<uintptr_t>(ptr) & 0x2)
#define IS_STACK_FRAME(ptr) (reinterpret_cast<uintptr_t>(ptr) & 0x4)
