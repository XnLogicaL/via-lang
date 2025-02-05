/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlmath.h"

namespace via::lib
{

void math_exp(State *V)
{
    const TValue &num = getargument(V, 0);

    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(0, "Number", ENUM_NAME(num.type)));

    TValue val = TValue(std::exp(num.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_log(State *V)
{
    const TValue &base = getargument(V, 0);
    const TValue &num = getargument(V, 1);

    LIB_ASSERT(checknumber(V, base), ARG_MISMATCH(0, "Number", ENUM_NAME(base.type)));
    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(1, "Number", ENUM_NAME(num.type)));

    TValue val = TValue(std::log(num.val_number) / std::log(base.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_log10(State *V)
{
    const TValue &num = getargument(V, 0);

    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(0, "Number", ENUM_NAME(num.type)));

    TValue val = TValue(std::log10(num.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_pow(State *V)
{
    const TValue &num = getargument(V, 0);
    const TValue &e = getargument(V, 1);

    LIB_ASSERT(checknumber(V, num), ARG_MISMATCH(0, "Number", ENUM_NAME(num.type)));
    LIB_ASSERT(checknumber(V, e), ARG_MISMATCH(1, "Number", ENUM_NAME(e.type)));

    TValue val = TValue(std::pow(num.val_number, e.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_cos(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::cos(t.val_number)); // Compute the cosine
    push(V, val);
    nativeret(V, 1);
}

void math_tan(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::tan(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_asin(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::asin(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_acos(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::acos(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_atan(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::atan(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_atan2(State *V)
{
    const TValue &t = getargument(V, 0);
    const TValue &x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    TValue val = TValue(std::atan2(t.val_number, x.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_sinh(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::sinh(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_cosh(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::cosh(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_tanh(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::tanh(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_abs(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::fabs(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_min(State *V)
{
    const TValue &t = getargument(V, 0);
    const TValue &x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    TValue val = TValue(std::min(t.val_number, x.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_max(State *V)
{
    const TValue &t = getargument(V, 0);
    const TValue &x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));
    LIB_ASSERT(checknumber(V, x), ARG_MISMATCH(1, "Number", ENUM_NAME(x.type)));

    TValue val = TValue(std::max(t.val_number, x.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_round(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::round(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_floor(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::floor(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void math_ceil(State *V)
{
    const TValue &t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, t), ARG_MISMATCH(0, "Number", ENUM_NAME(t.type)));

    TValue val = TValue(std::ceil(t.val_number));
    push(V, val);
    nativeret(V, 1);
}

void loadmathlib(State *) {}

} // namespace via::lib