/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "vm.h"
#include "types.h"
#include "libutils.h"

#include <cmath>

namespace via::VM::viastl
{

via_Table &vec3_new(VirtualMachine *vm,
    const via_Number x,
    const via_Number y,
    const via_Number z
);

void vec3_magnitude(VirtualMachine *vm)
{
    Register selfr = get_self_register();
    via_Table *self = vm->rget(selfr).tbl;

    // ! Assume self is a table
    via_Number mag = std::sqrt(
        std::pow(vm->tget(self, "x").num, 2) +
        std::pow(vm->tget(self, "y").num, 2) +
        std::pow(vm->tget(self, "z").num, 2)
    );

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(mag));
}

void vec3_normalize(VirtualMachine *vm)
{
    Register selfr = get_self_register();
    via_Table *self = vm->rget(selfr).tbl;

    // Use vm->call to ensure everything goes correctly
    vm->call(vm->tget(self, "magnitude"));

    Register rr = get_ret_register(0);
    // ! Assume mag is a number
    via_Number mag = vm->rget(rr).num;
    via_Table normal = vec3_new(vm,
        vm->tget(self, "x").num / mag,
        vm->tget(self, "y").num / mag,
        vm->tget(self, "z").num / mag
    );

    vm->rset(rr, via_Value(normal));
    return;
}

via_Table &vec3_new(VirtualMachine *vm,
    const via_Number x,
    const via_Number y,
    const via_Number z
)
{
    via_Table vec3_ins;

    LibConstructor::add_member(vec3_ins, "x", via_Value(x));
    LibConstructor::add_member(vec3_ins, "y", via_Value(y));
    LibConstructor::add_member(vec3_ins, "z", via_Value(z));

    LibConstructor::add_method(vec3_ins, "magnitude", vec3_magnitude);
    LibConstructor::add_method(vec3_ins, "normalize", vec3_normalize);
}

} // namespace via::VM::viastl
