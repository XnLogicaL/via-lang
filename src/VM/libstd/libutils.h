#pragma once

#include "vm.h"
#include "types.h"

via::VM::via_TableKey make_key(const char* k)
{
    return via::VM::via_TableKey {
        .type = via::VM::via_TableKey::KType::String,
        .str = const_cast<char*>(k)
    };
}

inline via::VM::Register get_arg_register(size_t roffset)
{
    return { via::VM::Register::RType::AR, roffset };
}

inline via::VM::Register get_ret_register(size_t roffset)
{
    return { via::VM::Register::RType::RR, roffset };
}

inline via::VM::Register get_self_register()
{
    return { via::VM::Register::RType::SELFR, 0 };
}

inline bool is_nil(const via::VM::via_Value &v)
{
    return v.type == via::VM::via_Value::VType::Nil;
}

namespace via::VM::LibConstructor
{

via_Table &new_lib()
{
    via_Table t;
    t.set(make_key("__type"), via_Value("Library"));
    return t;
}

void add_method(via_Table &l, const via_String &k, void(*f)(VirtualMachine *))
{
    l.set(make_key(k), via_Value(f));
}

void add_member(via_Table &l, const via_String k, via_Value v)
{
    l.set(make_key(k), v);
}

void seal(via_Table &l)
{
    l.is_frozen.set(true);
}

} // namespace via::VM::LibConstructor