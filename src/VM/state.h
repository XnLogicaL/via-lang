/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via::VM
{

enum class FFlag : uint8_t
{
    ABRT,
    SKIP
};

struct VMState
{
    bool is_running;
    int exit_code;
    std::string exit_message;

    std::bitset<32> fflags;

    VMState()
        : is_running(false)
        , exit_code(0)
        , exit_message("")
    {
    }
};

} // namespace via::VM
