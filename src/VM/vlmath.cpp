/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlmath.h"

namespace via::lib
{

void math_exp(RTState *V)
{
    TValue &num = getargument(V, 0);

    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(0, "Number", ENUM_NAME(num.type)));

    TValue val = TValue(std::exp(num.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_log(RTState *V)
{
    TValue &base = getargument(V, 0);
    TValue &num = getargument(V, 1);

    LIB_ASSERT(checknumber(V, base), ARG_MISMATCH(0, "Number", ENUM_NAME(base.type)));
    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(1, "Number", ENUM_NAME(num.type)));

    TValue val = TValue(std::log(num.val_number) / std::log(base.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_log10(RTState *V)
{
    TValue &num = getargument(V, 0);

    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(0, "Number", ENUM_NAME(num.type)));

    TValue val = TValue(std::log10(num.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_pow(RTState *V)
{
    TValue &num = getargument(V, 0);
    TValue &e = getargument(V, 1);

    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(0, "Number", ENUM_NAME(num.type)));
    LIB_ASSERT(checknumber(V, e), ARG_MISMATCH(1, "Number", ENUM_NAME(e.type)));

    TValue val = TValue(std::pow(num.val_number, e.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_cos(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::cos(t.val_number)); // Compute the cosine
    pushval(V, val);
    nativeret(V, 1);
}

void math_tan(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::tan(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_asin(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::asin(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_acos(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::acos(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_atan(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::atan(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_atan2(RTState *V)
{
    TValue &t = getargument(V, 0);
    TValue &x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    TValue val = TValue(std::atan2(t.val_number, x.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_sinh(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::sinh(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_cosh(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::cosh(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_tanh(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::tanh(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_abs(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::fabs(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_min(RTState *V)
{
    TValue &t = getargument(V, 0);
    TValue &x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    TValue val = TValue(std::min(t.val_number, x.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_max(RTState *V)
{
    TValue &t = getargument(V, 0);
    TValue &x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    TValue val = TValue(std::max(t.val_number, x.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_round(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::round(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_floor(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::floor(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_ceil(RTState *V)
{
    TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::ceil(t.val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void loadmathlib(RTState *) {}

} // namespace via::lib