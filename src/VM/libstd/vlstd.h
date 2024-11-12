#ifndef VIA_LIBSTD_H
#define VIA_LIBSTD_H

#include "vm.h"
#include "types.h"
#include "libutils.h"

#include "vlmath.h"

namespace via::VM::vstl
{

void std_print(VirtualMachine *vm)
{
    size_t i = 0;
    std::ostringstream oss;

    while (true)
    {
        Register r = { Register::RType::AR, i++ };
        via_Value rv = vm->rget(r);

        if (rv.type == via_Value::VType::Nil)
        {
            break;
        }

        oss << vm->vtostring(rv).str << " ";
    }
    
    std::cout << oss.str() << "\n";

    return;
}

void std_error(VirtualMachine *vm)
{
    Register r = get_arg_register(0);
    via_Value rv = vm->rget(r);

    vm->fatalerr(vm->vtostring(rv).str);

    return;
}

void std_exit(VirtualMachine *vm)
{
    Register r = get_arg_register(0);
    via_Value ec = vm->rget(r);

    bool assertion = ec.type != via_Value::VType::Number;

    vm->vm_assert(assertion,
        "Expected type Number for argument 0 of std_exit");

    if (!assertion)
        return;

    int code = static_cast<int>(ec.num);

    vm->set_exit_data(code, "std_exit called by user");
    vm->set_fflag("FFLAG_ABRT", true);

    return;
}

void std_type(VirtualMachine *vm)
{
    Register r = get_arg_register(0);
    via_Value rv = vm->rget(r);
    Register rr = get_ret_register(0);

    vm->rset(rr, vm->vtype(rv));

    return;
}

void std_typeof(VirtualMachine *vm)
{
    Register r = get_arg_register(0);
    via_Value rv = vm->rget(r);
    Register rr = get_ret_register(0);

    vm->rset(rr, vm->vtypeof(rv));

    return;
}

void std_tostring(VirtualMachine *vm)
{
    Register r = get_arg_register(0);
    via_Value rv = vm->rget(r);
    Register rr = get_ret_register(0);

    vm->rset(rr, vm->vtostring(rv));

    return;
}

void std_tonumber(VirtualMachine *vm)
{
    Register r = get_arg_register(0);
    via_Value rv = vm->rget(r);
    Register rr = get_ret_register(0);

    vm->rset(rr, vm->vtonumber(rv));

    return;
}

void std_tobool(VirtualMachine *vm)
{
    Register r = get_arg_register(0);
    via_Value rv = vm->rget(r);
    Register rr = get_ret_register(0);

    vm->rset(rr, vm->vtobool(rv));

    return;
}

void std_assert(VirtualMachine *vm)
{
    Register cr = get_arg_register(0);
    Register mr = get_arg_register(1);

    via_Value cv = vm->rget(cr);
    via_Value mv = vm->rget(mr);

    if (!vm->vtobool(cv).boole)
    {
        via_String mvstr = vm->vtostring(mv).str;
        std::string mfstr = std::format("std_assert assertion failed: {}", mvstr);
        via_String mfstrds = strdup(mfstr.c_str());

        vm->rset(cr, mfstrds);

        std_error(vm);
    }

    return;
}

void vstl_load(VirtualMachine *vm)
{
    // via base library v0.2.2
    vm->gset("print", via_Value(std_print));
    vm->gset("error", via_Value(std_error));
    vm->gset("exit", via_Value(std_exit));
    vm->gset("type", via_Value(std_type));
    vm->gset("typeof", via_Value(std_typeof));
    vm->gset("tostring", via_Value(std_tostring));
    vm->gset("tonumber", via_Value(std_tonumber));
    vm->gset("tobool", via_Value(std_tobool));
    vm->gset("assert", via_Value(std_assert));

    // other libs
    vm->gset("math", viastl::vstl_math_load(vm));
}

} // namespace via::VM::vstl

#endif // VIA_LIBSTD_H
