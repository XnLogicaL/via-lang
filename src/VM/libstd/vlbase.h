/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "libutils.h"
#include "shared.h"
#include "types.h"
#include "state.h"

namespace via::lib
{

inline void base_print(viaState *V)
{
    uint8_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i < V->argc)
    {
        viaRegister R = viaL_getargument(V, i++);
        viaValue RV = *via_getregister(V, R);

        oss << via_tostring(V, RV).str->ptr << " ";
    }

    std::cout << oss.str() << "\n"; // Output the accumulated string
}

inline void base_error(viaState *V)
{
    viaRegister R = viaL_getargument(V, 0);
    viaValue RV = *via_getregister(V, R);

    via_fatalerr(V, via_tostring(V, RV).str->ptr);
}

inline void base_exit(viaState *V)
{
    viaRegister R = viaL_getargument(V, 0);
    viaValue EC = *via_getregister(V, R);

    LIB_ASSERT(EC.type == viaValueType::Number, "Expected type viaNumber for argument 0 of base_exit");

    viaExitCode code = EC.num;

    via_setexitdata(V, code, "base_exit called by user");
    V->abrt = true; // Abort the VM execution
}

inline void base_type(viaState *V)
{
    viaRegister R = viaL_getargument(V, 0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, via_type(V, RV));
}

inline void base_typeof(viaState *V)
{
    viaRegister R = viaL_getargument(V, 0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, via_typeof(V, RV));
}

inline void base_tostring(viaState *V)
{
    viaRegister R = viaL_getargument(V, 0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, via_tostring(V, RV));
}

inline void base_tonumber(viaState *V)
{
    viaRegister R = viaL_getargument(V, 0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, via_tonumber(V, RV));
}

inline void base_tobool(viaState *V)
{
    viaRegister R = viaL_getargument(V, 0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, via_tobool(V, RV));
}

inline void base_assert(viaState *V)
{
    viaRegister cr = viaL_getargument(V, 0);
    viaRegister mr = viaL_getargument(V, 1);

    viaValue cv = *via_getregister(V, cr);
    viaValue mv = *via_getregister(V, mr);

    if (!via_tobool(V, cv).boole)
    {
        viaString *mvstr = via_tostring(V, mv).str;
        std::string mfstr = std::format("base_assert assertion failed: {}", mvstr->ptr);
        viaString *mfstrds = viaT_newstring(V, mfstr.c_str());

        via_setregister(V, cr, *mfstrds); // Set the error message in the first register

        base_error(V); // Invoke the error handler
    }
}

inline void viaL_loadbaselib(viaState *V)
{
    viaHashMap<viaRawString, viaValue> base_properties = {
        {"print", WRAPVAL(base_print)},
        {"error", WRAPVAL(base_error)},
        {"exit", WRAPVAL(base_exit)},
        {"type", WRAPVAL(base_type)},
        {"typeof", WRAPVAL(base_typeof)},
        {"tostring", WRAPVAL(base_tostring)},
        {"tonumber", WRAPVAL(base_tonumber)},
        {"tobool", WRAPVAL(base_tobool)},
        {"assert", WRAPVAL(base_assert)}
    };

    for (auto it : base_properties)
    {
        viaGlobalIdentifier id(it.first);
        via_setglobal(V, id, it.second);
    }
}

} // namespace via::lib
