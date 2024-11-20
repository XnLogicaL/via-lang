/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "vm.h"
#include "types.h"
#include "libutils.h"

#include <cmath>

namespace via::VM::viastl
{

viaTable &vec3_new(VirtualMachine *vm, const viaNumber x, const viaNumber y, const viaNumber z);

void vec3_magnitude(VirtualMachine *vm)
{
    viaRegister selfr = get_self_register();
    viaTable *self = vm->rget(selfr).tbl;

    // ! Assume self is a table
    viaNumber mag = std::sqrt(std::pow(vm->tget(self, "x").num, 2) + std::pow(vm->tget(self, "y").num, 2) + std::pow(vm->tget(self, "z").num, 2));

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(mag));
}

void vec3_normalize(VirtualMachine *vm)
{
    viaRegister selfr = get_self_register();
    viaTable *self = vm->rget(selfr).tbl;

    // Use vm->call to ensure everything goes correctly
    vm->call(vm->tget(self, "magnitude"));

    viaRegister rr = get_ret_register(0);
    // ! Assume mag is a number
    viaNumber mag = vm->rget(rr).num;
    viaTable normal = vec3_new(vm, vm->tget(self, "x").num / mag, vm->tget(self, "y").num / mag, vm->tget(self, "z").num / mag);

    vm->rset(rr, viaValue(normal));
    return;
}

viaTable &vec3_new(VirtualMachine *vm, const viaNumber x, const viaNumber y, const viaNumber z)
{
    viaTable vec3_ins;

    LibConstructor::add_member(vec3_ins, "x", viaValue(x));
    LibConstructor::add_member(vec3_ins, "y", viaValue(y));
    LibConstructor::add_member(vec3_ins, "z", viaValue(z));

    LibConstructor::add_method(vec3_ins, "magnitude", vec3_magnitude);
    LibConstructor::add_method(vec3_ins, "normalize", vec3_normalize);
}

} // namespace via::VM::viastl
