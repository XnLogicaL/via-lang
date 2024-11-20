/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "vm.h"
#include "types.h"
#include "libutils.h"

#include <cmath>

namespace via::VM::viastl
{

bool is_number(const viaValue &v)
{
    return v.type == viaValueType::viaNumber;
}

void math_exp(VirtualMachine *vm)
{
    viaRegister nr = get_arg_register(0);
    viaValue n = vm->rget(nr);

    vm->vm_assert(is_number(n), "Expected viaNumber for argument 0 of math_exp");

    // Unfortunately, this has to be done
    if (!is_number(n))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::exp(n.num)));

    return;
}

void math_log(VirtualMachine *vm)
{
    viaRegister br = get_arg_register(0);
    viaRegister nr = get_arg_register(1);

    viaValue base = vm->rget(br);
    viaValue n = vm->rget(nr);

    vm->vm_assert(is_number(base), "Expected viaNumber for argument 0 of math_log");
    vm->vm_assert(is_number(n), "Expected viaNumber for argument 1 of math_log");

    if (!is_number(base) || !is_number(n))
        return;

    viaRegister rr = get_ret_register(0);

    vm->rset(rr, viaValue(std::log(n.num) / std::log(base.num)));

    return;
}

void math_log10(VirtualMachine *vm)
{
    viaRegister nr = get_arg_register(0);
    viaValue n = vm->rget(nr);

    vm->vm_assert(is_number(n), "Expected viaNumber for argument 0 of math_log10");

    if (!is_number(n))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::log10(n.num)));

    return;
}

void math_pow(VirtualMachine *vm)
{
    viaRegister nr = get_arg_register(0);
    viaRegister er = get_arg_register(1);

    viaValue n = vm->rget(nr);
    viaValue e = vm->rget(er);

    vm->vm_assert(is_number(n), "Expected viaNumber for argument 0 of math_pow");
    vm->vm_assert(is_number(e), "Expected viaNumber for argument 1 of math_pow");

    if (!is_number(n) || !is_number(e))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::pow(n.num, e.num)));

    return;
}

void math_cos(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_cos");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::cos(t.num)));

    return;
}

void math_tan(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_tan");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::tan(t.num)));

    return;
}

void math_asin(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_asin");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::asin(t.num)));

    return;
}

void math_acos(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_acos");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::acos(t.num)));

    return;
}

void math_atan(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_atan");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::atan(t.num)));

    return;
}

void math_atan2(VirtualMachine *vm)
{
    viaRegister xr = get_arg_register(0);
    viaRegister yr = get_arg_register(1);

    viaValue x = vm->rget(xr);
    viaValue y = vm->rget(yr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_atan2");
    vm->vm_assert(is_number(y), "Expected number for argument 1 of math_atan2");

    if (!is_number(x) || !is_number(y))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::atan2(x.num, y.num)));

    return;
}

void math_sinh(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_sinh");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::sinh(t.num)));

    return;
}

void math_cosh(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_cosh");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::cosh(t.num)));

    return;
}

void math_tanh(VirtualMachine *vm)
{
    viaRegister tr = get_arg_register(0);
    viaValue t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_tanh");

    if (!is_number(t))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::tanh(t.num)));

    return;
}

void math_abs(VirtualMachine *vm)
{
    viaRegister xr = get_arg_register(0);
    viaValue x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_abs");

    if (!is_number(x))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::fabs(x.num)));

    return;
}

void math_min(VirtualMachine *vm)
{
    viaRegister xr = get_arg_register(0);
    viaRegister yr = get_arg_register(1);

    viaValue x = vm->rget(xr);
    viaValue y = vm->rget(yr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_min");
    vm->vm_assert(is_number(y), "Expected number for argument 1 of math_min");

    if (!is_number(x) || !is_number(y))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::min(x.num, y.num)));

    return;
}

void math_max(VirtualMachine *vm)
{
    viaRegister xr = get_arg_register(0);
    viaRegister yr = get_arg_register(1);

    viaValue x = vm->rget(xr);
    viaValue y = vm->rget(yr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_max");
    vm->vm_assert(is_number(y), "Expected number for argument 1 of math_max");

    if (!is_number(x) || !is_number(y))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::max(x.num, y.num)));

    return;
}

void math_round(VirtualMachine *vm)
{
    viaRegister xr = get_arg_register(0);
    viaValue x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_round");

    if (!is_number(x))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::round(x.num)));

    return;
}

void math_floor(VirtualMachine *vm)
{
    viaRegister xr = get_arg_register(0);
    viaValue x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_floor");

    if (!is_number(x))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::floor(x.num)));

    return;
}

void math_ceil(VirtualMachine *vm)
{
    viaRegister xr = get_arg_register(0);
    viaValue x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_ceil");

    if (!is_number(x))
        return;

    viaRegister rr = get_ret_register(0);
    vm->rset(rr, viaValue(std::ceil(x.num)));

    return;
}

void vstl_math_load(VirtualMachine *vm)
{
    viaTable std_math = LibConstructor::new_lib();

    LibConstructor::add_member(std_math, "pi", viaValue(3.1415926535));

    // Ah yes
    LibConstructor::add_method(std_math, "exp", math_exp);
    LibConstructor::add_method(std_math, "log", math_log);
    LibConstructor::add_method(std_math, "log10", math_log10);
    LibConstructor::add_method(std_math, "pow", math_pow);
    LibConstructor::add_method(std_math, "cos", math_cos);
    LibConstructor::add_method(std_math, "tan", math_tan);
    LibConstructor::add_method(std_math, "asin", math_asin);
    LibConstructor::add_method(std_math, "acos", math_acos);
    LibConstructor::add_method(std_math, "atan", math_atan);
    LibConstructor::add_method(std_math, "atan2", math_atan2);
    LibConstructor::add_method(std_math, "sinh", math_sinh);
    LibConstructor::add_method(std_math, "cosh", math_cosh);
    LibConstructor::add_method(std_math, "tanh", math_tanh);
    LibConstructor::add_method(std_math, "abs", math_abs);
    LibConstructor::add_method(std_math, "min", math_min);
    LibConstructor::add_method(std_math, "max", math_max);
    LibConstructor::add_method(std_math, "round", math_round);
    LibConstructor::add_method(std_math, "floor", math_floor);
    LibConstructor::add_method(std_math, "ceil", math_ceil);

    LibConstructor::seal(std_math);

    viaValue std_math_v;
    std_math_v.is_const = true;

    vm->loadlib("math", std_math_v);

    return;
}

} // namespace via::VM::viastl
