/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlfunction.h"

namespace via::lib
{

void function_name(State *V)
{
    TValue &func = getargument(V, 0);

    // Check if the argument is a function
    // If so return nil
    if (!checkfunction(V, func))
    {
        push(V, nil);
        return;
    }

    TValue id = TValue(func.val_function->id);
    push(V, id);
    nativeret(V, 1);
}

} // namespace via::lib