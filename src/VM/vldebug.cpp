/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vldebug.h"
#include "libutils.h"

namespace via
{

void debug_traceback(RTState *V)
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

void loaddebuglib(RTState *V)
{
    static const HashMap<const char *, TValue> debug_properties = {
        {"traceback", WRAPVAL(debug_traceback)},
    };

    TTable *lib = new TTable();

    for (const auto &[ident, val] : debug_properties)
        settableindex(V, lib, hashstring(V, ident), val);

    freeze(V, lib);
    setglobal(V, "debug", TValue(lib));
}

} // namespace via