#include "vm.h"
#include "types.h"
#include "libutils.h"

#include <cmath>

namespace via::VM::viastl
{

bool is_number(const via_Value &v)
{
    return v.type == via_Value::VType::Number;
}

void math_exp(VirtualMachine *vm)
{
    Register nr = get_arg_register(0);
    via_Value n = vm->rget(nr);

    vm->vm_assert(is_number(n),
        "Expected Number for argument 0 of math_exp");

    // Unfortunately, this has to be done
    if (!is_number(n))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::exp(n.num)));

    return;
}

void math_log(VirtualMachine *vm)
{
    Register br = get_arg_register(0);
    Register nr = get_arg_register(1);

    via_Value base = vm->rget(br);
    via_Value n = vm->rget(nr);

    vm->vm_assert(is_number(base), "Expected Number for argument 0 of math_log");
    vm->vm_assert(is_number(n), "Expected Number for argument 1 of math_log");

    if (!is_number(base) || !is_number(n))
        return;

    Register rr = get_ret_register(0);

    vm->rset(rr, via_Value(std::log(n.num) / std::log(base.num)));

    return;
}

void math_log10(VirtualMachine *vm)
{
    Register nr = get_arg_register(0);
    via_Value n = vm->rget(nr);

    vm->vm_assert(is_number(n), "Expected Number for argument 0 of math_log10");
    
    if (!is_number(n))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::log10(n.num)));

    return;
}

void math_pow(VirtualMachine *vm)
{
    Register nr = get_arg_register(0);
    Register er = get_arg_register(1);

    via_Value n = vm->rget(nr);
    via_Value e = vm->rget(er);

    vm->vm_assert(is_number(n), "Expected Number for argument 0 of math_pow");
    vm->vm_assert(is_number(e), "Expected Number for argument 1 of math_pow");

    if (!is_number(n) || !is_number(e))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::pow(n.num, e.num)));

    return;
}

void math_cos(VirtualMachine *vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_cos");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::cos(t.num)));

    return;
}

void math_tan(VirtualMachine* vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_tan");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::tan(t.num)));

    return;
}

void math_asin(VirtualMachine* vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_asin");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::asin(t.num)));

    return;
}

void math_acos(VirtualMachine* vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_acos");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::acos(t.num)));

    return;
}

void math_atan(VirtualMachine* vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_atan");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::atan(t.num)));

    return;
}

void math_atan2(VirtualMachine* vm)
{
    Register xr = get_arg_register(0);
    Register yr = get_arg_register(1);

    via_Value x = vm->rget(xr);
    via_Value y = vm->rget(yr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_atan2");
    vm->vm_assert(is_number(y), "Expected number for argument 1 of math_atan2");

    if (!is_number(x) || !is_number(y))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::atan2(x.num, y.num)));

    return;
}

void math_sinh(VirtualMachine* vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_sinh");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::sinh(t.num)));

    return;
}

void math_cosh(VirtualMachine* vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_cosh");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::cosh(t.num)));

    return;
}

void math_tanh(VirtualMachine* vm)
{
    Register tr = get_arg_register(0);
    via_Value t = vm->rget(tr);

    vm->vm_assert(is_number(t), "Expected number for argument 0 of math_tanh");

    if (!is_number(t))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::tanh(t.num)));

    return;
}

void math_abs(VirtualMachine *vm)
{
    Register xr = get_arg_register(0);
    via_Value x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_abs");

    if (!is_number(x))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::fabs(x.num)));

    return;
}

void math_min(VirtualMachine *vm)
{
    Register xr = get_arg_register(0);
    Register yr = get_arg_register(1);

    via_Value x = vm->rget(xr);
    via_Value y = vm->rget(yr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_min");
    vm->vm_assert(is_number(y), "Expected number for argument 1 of math_min");

    if (!is_number(x) || !is_number(y))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::min(x.num, y.num)));

    return;
}

void math_max(VirtualMachine* vm)
{
    Register xr = get_arg_register(0);
    Register yr = get_arg_register(1);

    via_Value x = vm->rget(xr);
    via_Value y = vm->rget(yr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_max");
    vm->vm_assert(is_number(y), "Expected number for argument 1 of math_max");

    if (!is_number(x) || !is_number(y))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::max(x.num, y.num)));

    return;
}

void math_round(VirtualMachine* vm)
{
    Register xr = get_arg_register(0);
    via_Value x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_round");

    if (!is_number(x))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::round(x.num)));

    return;
}

void math_floor(VirtualMachine* vm)
{
    Register xr = get_arg_register(0);
    via_Value x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_floor");

    if (!is_number(x))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::floor(x.num)));

    return;
}

void math_ceil(VirtualMachine* vm)
{
    Register xr = get_arg_register(0);
    via_Value x = vm->rget(xr);

    vm->vm_assert(is_number(x), "Expected number for argument 0 of math_ceil");

    if (!is_number(x))
        return;

    Register rr = get_ret_register(0);
    vm->rset(rr, via_Value(std::ceil(x.num)));

    return;
}

via_Value &vstl_math_load(VirtualMachine *vm)
{
    via_Table std_math = LibConstructor::new_lib();

    LibConstructor::add_member(std_math, "pi", via_Value(3.1415926535));

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

    via_Value std_math_v;
    std_math_v.is_const = true;

    return std_math_v;
}

} // namespace via::VM::viastl
