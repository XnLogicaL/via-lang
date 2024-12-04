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
    viaValue n = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, n), ARG_MISMATCH(0, "Number", ENUM_NAME(n.type)));

    viaValue val = viaT_stackvalue(V, std::exp(nval_number));
    via_pushreturn(V, val);
}

inline void math_log(viaState *V)
{
    viaValue base = via_popargument(V);
    viaValue n = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, base), ARG_MISMATCH(0, "Number", ENUM_NAME(base.type)));
    LIB_ASSERT(viaT_checknumber(V, n), ARG_MISMATCH(1, "Number", ENUM_NAME(n.type)));

    viaValue val = viaT_stackvalue(V, std::log(nval_number) / std::log(baseval_number));
    via_pushreturn(V, val);
}

inline void math_log10(viaState *V)
{
    viaValue n = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, n), ARG_MISMATCH(0, "Number", ENUM_NAME(n.type)));

    viaValue val = viaT_stackvalue(V, std::log10(nval_number));
    via_pushreturn(V, val);
}

inline void math_pow(viaState *V)
{
    viaValue n = via_popargument(V);
    viaValue e = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, n), ARG_MISMATCH(0, "Number", ENUM_NAME(n.type)));
    LIB_ASSERT(viaT_checknumber(V, e), ARG_MISMATCH(1, "Number", ENUM_NAME(e.type)));

    viaValue val = viaT_stackvalue(V, std::pow(nval_number, eval_number));
    via_pushreturn(V, val);
}

inline void math_cos(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::cos(tval_number)); // Compute the cosine
    via_pushreturn(V, val);
}

inline void math_tan(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::tan(tval_number));
    via_pushreturn(V, val);
}

inline void math_asin(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::asin(tval_number));
    via_pushreturn(V, val);
}

inline void math_acos(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::acos(tval_number));
    via_pushreturn(V, val);
}

inline void math_atan(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::atan(tval_number));
    via_pushreturn(V, val);
}

inline void math_atan2(viaState *V)
{
    viaValue t = via_popargument(V);
    viaValue x = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(viaT_checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    viaValue val = viaT_stackvalue(V, std::atan2(tval_number, xval_number));
    via_pushreturn(V, val);
}

inline void math_sinh(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::sinh(tval_number));
    via_pushreturn(V, val);
}

inline void math_cosh(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::cosh(tval_number));
    via_pushreturn(V, val);
}

inline void math_tanh(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::atan(tval_number));
    via_pushreturn(V, val);
}

inline void math_abs(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::fabs(tval_number));
    via_pushreturn(V, val);
}

inline void math_min(viaState *V)
{
    viaValue t = via_popargument(V);
    viaValue x = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(viaT_checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    viaValue val = viaT_stackvalue(V, std::min(tval_number, xval_number));
    via_pushreturn(V, val);
}

inline void math_max(viaState *V)
{
    viaValue t = via_popargument(V);
    viaValue x = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(viaT_checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    viaValue val = viaT_stackvalue(V, std::max(tval_number, xval_number));
    via_pushreturn(V, val);
}

inline void math_round(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::round(tval_number));
    via_pushreturn(V, val);
}

inline void math_floor(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::floor(tval_number));
    via_pushreturn(V, val);
}

inline void math_ceil(viaState *V)
{
    viaValue t = via_popargument(V);

    LIB_ASSERT(viaT_checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    viaValue val = viaT_stackvalue(V, std::ceil(tval_number));
    via_pushreturn(V, val);
}

inline void viaL_loadmathlib(viaState *V)
{
    static const viaHashMap_t<viaRawString_t, viaValue> math_properties = {

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
    via_loadlib(V, "math", viaT_stackvalue(V, lib));
}

} // namespace via::lib