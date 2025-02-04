/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vldebug.h"
#include "libutils.h"

namespace via
{

void debug_traceback(State *V)
{
    TFunction *frame = V->frame;
    std::ostringstream oss;

    while (frame)
    {
        TValue copy = TValue(frame);
        tostring(V, copy);

        oss << std::format("{}:{}\n", copy.val_string->ptr, frame->line);
        frame = frame->caller;
    }

    TValue str(new TString(V, oss.str().c_str()));
    pushval(V, str);
}

void loaddebuglib(State *) {}

} // namespace via