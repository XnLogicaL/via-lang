/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlfs.h"

namespace via::lib
{

void fs_read(viaState *V)
{
    viaValue path = via_popargument(V);

    LIB_ASSERT(viaT_checkstring(V, path), ARG_MISMATCH(0, "String", ENUM_NAME(path.type)));

    std::ifstream f(path.val_string->ptr);
    std::string buf;
    std::string aux;

    while (std::getline(f, aux))
        buf += aux;

    f.close();

    viaValue val = viaT_stackvalue(V, viaT_newstring(V, buf.c_str()));
    via_pushreturn(V, val);
}

} // namespace via::lib