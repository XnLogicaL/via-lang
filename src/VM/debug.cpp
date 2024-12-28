/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "debug.h"

namespace via
{

void dbgprintregistermap(RTState *V = nullptr, size_t count = VIA_REGISTERCOUNT)
{
    for (GPRegister reg = 0; reg <= count; reg++)
    {
        TValue *val = rgetregister(V->ralloc, reg);
        TValue val_clone = *val;
        tostring(V, val_clone);
        std::string fmt = std::format("R{}<'{}':'{}'@{}>", reg, ENUM_NAME(val->type), val_clone.val_string->ptr, reinterpret_cast<const void *>(val));
        std::cout << fmt << "\n";
    }
}

void dbgprintargumentstack(RTState *V, size_t depth)
{
    if (V->arguments->size == 0)
    {
        std::cout << "<argument stack empty>\n";
        return;
    }

    size_t current_depth = 0;

    for (TValue argument : *V->arguments)
    {
        if (current_depth == depth)
            break;

        TValue arg_clone = argument;
        tostring(V, arg_clone);
        std::string fmt = std::format("Arg{}<'{}':'{}'>", current_depth, ENUM_NAME(argument.type), arg_clone.val_string->ptr);
        std::cout << fmt << "\n";

        current_depth++;
    }
}

void dbgprintreturnstack(RTState *V, size_t depth)
{
    if (V->arguments->size == 0)
    {
        std::cout << "<return stack empty>\n";
        return;
    }

    size_t current_depth = 0;

    for (TValue returnv : *V->returns)
    {
        if (current_depth == depth)
            break;

        TValue ret_clone = returnv;
        tostring(V, ret_clone);
        std::string fmt = std::format("Return{}<'{}':'{}'>", current_depth, ENUM_NAME(returnv.type), ret_clone.val_string->ptr);
        std::cout << fmt << "\n";

        current_depth++;
    }
}

void dbgprintcallstack(RTState *, size_t) {}

} // namespace via