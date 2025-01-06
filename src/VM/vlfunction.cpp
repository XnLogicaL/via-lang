/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlfunction.h"

namespace via::lib
{

void function_name(RTState *V)
{
    TValue *func = getargument(V, 0);

    // Check if the argument is a function
    // If so return nil
    if (!checkfunction(V, *func))
    {
        pushval(V, stackvalue(V));
        return;
    }

    TValue id = stackvalue(V, func->val_function->id);
    pushval(V, id);
}

} // namespace via::lib