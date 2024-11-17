/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "vm.h"
#include "types.h"
#include "libutils.h"
#include "common.h"

#include <filesystem>

namespace via::VM::viastl
{

void fs_read(VirtualMachine *vm)
{
    Register pr = get_arg_register(0);
    via_Value p = vm->rget(pr);

    vm->vm_assert(p.type == ValueType::String,
        "Expected String for argument 0 of fs_read");

    std::ifstream f(p.str);
    std::string buf;
    std::string aux;

    while (std::getline(f, aux))
    {
        buf += aux;
    }

    f.close();
    
    via_Value v = via_Value(strdup(buf.c_str()));
    Register rr = get_ret_register(0);
    
    vm->rset(rr, v);

    return;
}

} // namespace via::VM::viastl
