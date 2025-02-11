/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via
{

enum class VMExitCode
{
    success,
    force_abort,
    unknown_opcode,
    illegal_instruction_access,
    invalid_constant_index,
    invalid_string_access,
    invalid_table_access,
    attempt_call_non_callable,
    attempt_mutate_frozen_table,
};

}
