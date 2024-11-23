/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "shared.h"
#include "state.h"
#include "types.h"
#include "libutils.h"

#include <cmath>

namespace via::lib
{

inline void math_exp(viaState *V)
{
    viaRegister nR = viaL_getargument(V, 0);
    viaValue n = *via_getregister(V, nR);

    LIB_ASSERT(viaT_checknumber(V, n), "Expected Number for argument 0 of math_exp");

    viaValue val = viaT_stackvalue(V, std::exp(n.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_log(viaState *V)
{
    viaRegister bR = viaL_getargument(V, 0);
    viaRegister nR = viaL_getreturn(V, 1);

    viaValue base = *via_getregister(V, bR);
    viaValue n = *via_getregister(V, nR);

    LIB_ASSERT(viaT_checknumber(V, base), "Expected Number for argument 0 of math_log");
    LIB_ASSERT(viaT_checknumber(V, n), "Expected Number for argument 1 of math_log");

    viaValue val = viaT_stackvalue(V, std::log(n.num) / std::log(base.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_log10(viaState *V)
{
    viaRegister nR = viaL_getargument(V, 0);
    viaValue n = *via_getregister(V, nR);

    LIB_ASSERT(viaT_checknumber(V, n), "Expected Number for argument 0 of math_log10");

    viaValue val = viaT_stackvalue(V, std::log10(n.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_pow(viaState *V)
{
    viaRegister nR = viaL_getargument(V, 0);
    viaRegister eR = viaL_getargument(V, 1);

    viaValue n = *via_getregister(V, nR);
    viaValue e = *via_getregister(V, eR);

    LIB_ASSERT(viaT_checknumber(V, n), "Expected Number for argument 0 of math_pow");
    LIB_ASSERT(viaT_checknumber(V, e), "Expected Number for argument 1 of math_pow");

    viaValue val = viaT_stackvalue(V, std::pow(n.num, e.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_cos(viaState *V)
{
    viaRegister tR = viaL_getargument(V, 0); // Get the argument register
    viaValue t = *via_getregister(V, tR);    // Retrieve the value from the register

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_cos");

    viaValue val = viaT_stackvalue(V, std::cos(t.num)); // Compute the cosine
    viaRegister RR = viaL_getreturn(V, 0);              // Get the return register

    via_setregister(V, RR, val); // Set the return value
}

inline void math_tan(viaState *V)
{
    viaRegister tR = viaL_getargument(V, 0);
    viaValue t = *via_getregister(V, tR);

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_tan");

    viaValue val = viaT_stackvalue(V, std::tan(t.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_asin(viaState *V)
{
    viaRegister tR = viaL_getargument(V, 0);
    viaValue t = *via_getregister(V, tR);

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_asin");

    viaValue val = viaT_stackvalue(V, std::asin(t.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_acos(viaState *V)
{
    viaRegister tR = viaL_getargument(V, 0);
    viaValue t = *via_getregister(V, tR);

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_acos");

    viaValue val = viaT_stackvalue(V, std::acos(t.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_atan(viaState *V)
{
    viaRegister tR = viaL_getargument(V, 0);
    viaValue t = *via_getregister(V, tR);

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_atan");

    viaValue val = viaT_stackvalue(V, std::atan(t.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_atan2(viaState *V)
{
    viaRegister xR = viaL_getargument(V, 0);
    viaRegister yR = viaL_getargument(V, 1);

    viaValue x = *via_getregister(V, xR);
    viaValue y = *via_getregister(V, yR);

    LIB_ASSERT(viaT_checknumber(V, x), "Expected Number for argument 0 of math_atan2");
    LIB_ASSERT(viaT_checknumber(V, y), "Expected Number for argument 1 of math_atan2");

    viaValue val = viaT_stackvalue(V, std::atan2(x.num, y.num));
    viaRegister RR = viaL_getreturn(V, 0);

    via_setregister(V, RR, val);
}

inline void math_sinh(viaState *V)
{
    viaRegister tr = viaL_getargument(V, 0);
    viaValue t = *via_getregister(V, tr);

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_sinh");

    viaValue result = viaT_stackvalue(V, std::sinh(t.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_cosh(viaState *V)
{
    viaRegister tr = viaL_getargument(V, 0);
    viaValue t = *via_getregister(V, tr);

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_cosh");

    viaValue result = viaT_stackvalue(V, std::cosh(t.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_tanh(viaState *V)
{
    viaRegister tr = viaL_getargument(V, 0);
    viaValue t = *via_getregister(V, tr);

    LIB_ASSERT(viaT_checknumber(V, t), "Expected Number for argument 0 of math_tanh");

    viaValue result = viaT_stackvalue(V, std::tanh(t.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_abs(viaState *V)
{
    viaRegister xr = viaL_getargument(V, 0);
    viaValue x = *via_getregister(V, xr);

    LIB_ASSERT(viaT_checknumber(V, x), "Expected Number for argument 0 of math_abs");

    viaValue result = viaT_stackvalue(V, std::fabs(x.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_min(viaState *V)
{
    viaRegister xr = viaL_getargument(V, 0);
    viaRegister yr = viaL_getargument(V, 1);

    viaValue x = *via_getregister(V, xr);
    viaValue y = *via_getregister(V, yr);

    LIB_ASSERT(viaT_checknumber(V, x), "Expected Number for argument 0 of math_min");
    LIB_ASSERT(viaT_checknumber(V, y), "Expected Number for argument 1 of math_min");

    viaValue result = viaT_stackvalue(V, std::min(x.num, y.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_max(viaState *V)
{
    viaRegister xr = viaL_getargument(V, 0);
    viaRegister yr = viaL_getargument(V, 1);

    viaValue x = *via_getregister(V, xr);
    viaValue y = *via_getregister(V, yr);

    LIB_ASSERT(viaT_checknumber(V, x), "Expected Number for argument 0 of math_max");
    LIB_ASSERT(viaT_checknumber(V, y), "Expected Number for argument 1 of math_max");

    viaValue result = viaT_stackvalue(V, std::max(x.num, y.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_round(viaState *V)
{
    viaRegister xr = viaL_getargument(V, 0);
    viaValue x = *via_getregister(V, xr);

    LIB_ASSERT(viaT_checknumber(V, x), "Expected Number for argument 0 of math_round");

    viaValue result = viaT_stackvalue(V, std::round(x.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_floor(viaState *V)
{
    viaRegister xr = viaL_getargument(V, 0);
    viaValue x = *via_getregister(V, xr);

    LIB_ASSERT(viaT_checknumber(V, x), "Expected Number for argument 0 of math_floor");

    viaValue result = viaT_stackvalue(V, std::floor(x.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void math_ceil(viaState *V)
{
    viaRegister xr = viaL_getargument(V, 0);
    viaValue x = *via_getregister(V, xr);

    LIB_ASSERT(viaT_checknumber(V, x), "Expected Number for argument 0 of math_ceil");

    viaValue result = viaT_stackvalue(V, std::ceil(x.num));
    viaRegister rr = viaL_getreturn(V, 0);

    via_setregister(V, rr, result);
}

inline void viaL_loadmathlib(viaState *V)
{
    static const viaHashMap<viaRawString, viaValue> math_properties = {

        // Constants
        {"pi", WRAPVAL(3.1415926535)},

        // Functions
        {"exp", WRAPVAL(math_exp)},
        {"log", WRAPVAL(math_log)},
        {"log10", WRAPVAL(math_log10)},
        {"pow", WRAPVAL(math_pow)},
        {"cos", WRAPVAL(math_cos)},
        {"tan", WRAPVAL(math_tan)},
        {"asin", WRAPVAL(math_asin)},
        {"acos", WRAPVAL(math_acos)},
        {"atan", WRAPVAL(math_atan)},
        {"atan2", WRAPVAL(math_atan2)},
        {"sinh", WRAPVAL(math_sinh)},
        {"cosh", WRAPVAL(math_cosh)},
        {"tanh", WRAPVAL(math_tanh)},
        {"abs", WRAPVAL(math_abs)},
        {"min", WRAPVAL(math_min)},
        {"max", WRAPVAL(math_max)},
        {"round", WRAPVAL(math_round)},
        {"floor", WRAPVAL(math_floor)},
        {"ceil", WRAPVAL(math_ceil)}
    };

    viaTable *lib = viaT_newtable(V, nullptr, {});

    for (auto it : math_properties)
    {
        viaTableKey key = viaT_hashstring(V, it.first);
        via_settableindex(V, lib, key, it.second);
    }

    via_freeze(V, lib);

    via_freeze(V, lib);
    via_loadlib(V, "math", viaT_stackvalue(V, lib));
}

} // namespace via::lib
