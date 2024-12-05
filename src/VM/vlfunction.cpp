/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlfunction.h"

namespace via::lib
{

void function_name(viaState *V)
{
    viaValue func = via_popargument(V);

    // Check if the argument is a function
    // If so return nil
    if (!viaT_checkfunction(V, func))
    {
        via_pushreturn(V, viaT_stackvalue(V));
        return;
    }

    via_pushreturn(V, viaT_stackvalue(V, func.val_function->id));
}

} // namespace via::lib