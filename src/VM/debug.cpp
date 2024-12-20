/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "debug.h"

namespace via
{

void viaD_printregistermap(viaState *V = nullptr, size_t count = VIA_REGISTERCOUNT)
{
    for (Register reg = 0; reg <= count; reg++)
    {
        viaValue *val = viaR_getregister(V->ralloc, reg);
        viaValue val_clone = *val;
        via_tostring(V, val_clone);
        std::string fmt = std::format("R{}<'{}':'{}'@{}>", reg, ENUM_NAME(val->type), val_clone.val_string->ptr, reinterpret_cast<const void *>(val));
        std::cout << fmt << "\n";
    }
}

void viaD_printargumentstack(viaState *V, size_t depth)
{
    if (V->arguments->size == 0)
    {
        std::cout << "<argument stack empty>\n";
        return;
    }

    size_t current_depth = 0;

    for (viaValue argument : *V->arguments)
    {
        if (current_depth == depth)
            break;

        viaValue arg_clone = argument;
        via_tostring(V, arg_clone);
        std::string fmt = std::format("Arg{}<'{}':'{}'>", current_depth, ENUM_NAME(argument.type), arg_clone.val_string->ptr);
        std::cout << fmt << "\n";

        current_depth++;
    }
}

void viaD_printreturnstack(viaState *V, size_t depth)
{
    if (V->arguments->size == 0)
    {
        std::cout << "<return stack empty>\n";
        return;
    }

    size_t current_depth = 0;

    for (viaValue returnv : *V->returns)
    {
        if (current_depth == depth)
            break;

        viaValue ret_clone = returnv;
        via_tostring(V, ret_clone);
        std::string fmt = std::format("Return{}<'{}':'{}'>", current_depth, ENUM_NAME(returnv.type), ret_clone.val_string->ptr);
        std::cout << fmt << "\n";

        current_depth++;
    }
}

void viaD_printcallstack(viaState *, size_t) {}

} // namespace via